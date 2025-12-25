extern "C" {
#include "../libs/libgrapheme/grapheme.h"
}
#include "../include/editor.h"
#include "../include/lsp.h"
#include "../include/main.h"
#include "../include/utils.h"

uint32_t scan_left(const char *line, uint32_t len, uint32_t off) {
  if (off > len)
    off = len;
  uint32_t i = off;
  while (i > 0) {
    unsigned char c = (unsigned char)line[i - 1];
    if ((c & 0x80) != 0)
      break;
    if (!((c == '_') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
          (c >= 'a' && c <= 'z')))
      break;
    --i;
  }
  return i;
}

uint32_t scan_right(const char *line, uint32_t len, uint32_t off) {
  if (off > len)
    off = len;
  uint32_t i = off;
  while (i < len) {
    unsigned char c = (unsigned char)line[i];
    if ((c & 0x80) != 0)
      break;
    if (!((c == '_') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
          (c >= 'a' && c <= 'z')))
      break;
    ++i;
  }
  return i;
}

void word_boundaries_exclusive(Editor *editor, Coord coord, uint32_t *prev_col,
                               uint32_t *next_col) {
  if (!editor)
    return;
  std::shared_lock lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, coord.row);
  if (!it)
    return;
  uint32_t line_len;
  char *line = next_line(it, &line_len);
  if (!line)
    return;
  if (line_len && line[line_len - 1] == '\n')
    line_len--;
  uint32_t col = coord.col;
  if (col > line_len)
    col = line_len;
  uint32_t left = scan_left(line, line_len, col);
  uint32_t right = scan_right(line, line_len, col);
  if (prev_col)
    *prev_col = left;
  if (next_col)
    *next_col = right;
  free(it->buffer);
  free(it);
}

uint32_t word_jump_right(const char *line, size_t len, uint32_t pos) {
  if (pos >= len)
    return len;
  size_t next = grapheme_next_word_break_utf8(line + pos, len - pos);
  return static_cast<uint32_t>(pos + next);
}

uint32_t word_jump_left(const char *line, size_t len, uint32_t col) {
  if (col == 0)
    return 0;
  size_t pos = 0;
  size_t last = 0;
  size_t cursor = col;
  while (pos < len) {
    size_t next = pos + grapheme_next_word_break_utf8(line + pos, len - pos);
    if (next >= cursor)
      break;
    last = next;
    pos = next;
  }
  return static_cast<uint32_t>(last);
}

void word_boundaries(Editor *editor, Coord coord, uint32_t *prev_col,
                     uint32_t *next_col, uint32_t *prev_clusters,
                     uint32_t *next_clusters) {
  if (!editor)
    return;
  std::shared_lock lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, coord.row);
  if (!it)
    return;
  uint32_t line_len;
  char *line = next_line(it, &line_len);
  if (!line)
    return;
  if (line_len && line[line_len - 1] == '\n')
    line_len--;
  size_t col = coord.col;
  if (col > line_len)
    col = line_len;
  size_t left = word_jump_left(line, line_len, col);
  size_t right = word_jump_right(line, line_len, col);
  if (prev_col)
    *prev_col = static_cast<uint32_t>(left);
  if (next_col)
    *next_col = static_cast<uint32_t>(right);
  if (prev_clusters)
    *prev_clusters = count_clusters(line, line_len, left, col);
  if (next_clusters)
    *next_clusters = count_clusters(line, line_len, col, right);
  free(it->buffer);
  free(it);
}

Coord editor_hit_test(Editor *editor, uint32_t x, uint32_t y) {
  if (mode == INSERT)
    x++;
  uint32_t numlen =
      EXTRA_META + static_cast<int>(std::log10(editor->root->line_count + 1));
  bool is_gutter_click = (x < numlen);
  uint32_t render_width = editor->size.col - numlen;
  x = MAX(x, numlen) - numlen;
  uint32_t target_visual_row = y;
  uint32_t visual_row = 0;
  uint32_t line_index = editor->scroll.row;
  uint32_t last_line_index = editor->scroll.row;
  uint32_t last_col = editor->scroll.col;
  bool first_visual_line = true;
  std::shared_lock knot_lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, line_index);
  if (!it)
    return editor->scroll;
  while (visual_row <= target_visual_row) {
    const Fold *fold = fold_for_line(editor->folds, line_index);
    if (fold) {
      if (visual_row == target_visual_row) {
        free(it->buffer);
        free(it);
        if (is_gutter_click) {
          remove_fold(editor, fold->start);
          return {UINT32_MAX, UINT32_MAX};
        }
        return {fold->start > 0 ? fold->start - 1 : 0, 0};
      }
      visual_row++;
      while (line_index <= fold->end) {
        char *l = next_line(it, nullptr);
        if (!l)
          break;
        line_index++;
      }
      last_line_index = fold->end;
      last_col = 0;
      continue;
    }
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line)
      break;
    if (line_len && line[line_len - 1] == '\n')
      line_len--;
    last_line_index = line_index;
    last_col = line_len;
    uint32_t offset = first_visual_line ? editor->scroll.col : 0;
    first_visual_line = false;
    while (offset < line_len || (line_len == 0 && offset == 0)) {
      uint32_t col = 0;
      uint32_t advance = 0;
      uint32_t left = line_len - offset;
      uint32_t last_good_offset = offset;
      while (left > 0 && col < render_width) {
        uint32_t g =
            grapheme_next_character_break_utf8(line + offset + advance, left);
        int w = display_width(line + offset + advance, g);
        if (col + w > render_width)
          break;
        if (visual_row == target_visual_row && x < col + w) {
          free(it->buffer);
          free(it);
          return {line_index, offset + advance};
        }
        advance += g;
        last_good_offset = offset + advance;
        left -= g;
        col += w;
      }
      last_col = last_good_offset;
      if (visual_row == target_visual_row) {
        free(it->buffer);
        free(it);
        return {line_index, last_good_offset};
      }
      visual_row++;
      if (visual_row > target_visual_row)
        break;
      if (advance == 0)
        break;
      offset += advance;
      if (line_len == 0)
        break;
    }
    line_index++;
  }
  free(it->buffer);
  free(it);
  return {last_line_index, last_col};
}

void move_line_up(Editor *editor) {
  if (!editor || !editor->root || editor->cursor.row == 0)
    return;
  if (mode == NORMAL || mode == INSERT) {
    uint32_t line_len, line_cluster_len;
    std::shared_lock lock(editor->knot_mtx);
    LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
    char *line = next_line(it, &line_len);
    if (!line) {
      lock.unlock();
      return;
    }
    if (line_len > 0 && line[line_len - 1] == '\n')
      line_len--;
    line_cluster_len = count_clusters(line, line_len, 0, line_len);
    uint32_t target_row = prev_unfolded_row(editor, editor->cursor.row - 1);
    uint32_t up_by = editor->cursor.row - target_row;
    if (up_by > 1)
      up_by--;
    lock.unlock();
    Coord cursor = editor->cursor;
    edit_erase(editor, {cursor.row, 0}, line_cluster_len);
    edit_erase(editor, {cursor.row, 0}, -1);
    edit_insert(editor, {cursor.row - up_by, 0}, (char *)"\n", 1);
    edit_insert(editor, {cursor.row - up_by, 0}, line, line_len);
    free(it->buffer);
    free(it);
    editor->cursor = {cursor.row - up_by, cursor.col};
  } else if (mode == SELECT) {
    uint32_t start_row = MIN(editor->cursor.row, editor->selection.row);
    uint32_t end_row = MAX(editor->cursor.row, editor->selection.row);
    uint32_t start_byte = line_to_byte(editor->root, start_row, nullptr);
    uint32_t end_byte = line_to_byte(editor->root, end_row + 1, nullptr);
    char *selected_text = read(editor->root, start_byte, end_byte - start_byte);
    if (!selected_text)
      return;
    uint32_t selected_len = count_clusters(selected_text, end_byte - start_byte,
                                           0, end_byte - start_byte);
    Coord cursor = editor->cursor;
    Coord selection = editor->selection;
    edit_erase(editor, {start_row, 0}, selected_len);
    edit_insert(editor, {start_row - 1, 0}, selected_text,
                end_byte - start_byte);
    free(selected_text);
    editor->cursor = {cursor.row - 1, cursor.col};
    editor->selection = {selection.row - 1, selection.col};
  }
}

void move_line_down(Editor *editor) {
  if (!editor || !editor->root)
    return;
  if (mode == NORMAL || mode == INSERT) {
    if (editor->cursor.row >= editor->root->line_count - 1)
      return;
    uint32_t line_len, line_cluster_len;
    std::shared_lock lock(editor->knot_mtx);
    LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
    char *line = next_line(it, &line_len);
    if (!line) {
      lock.unlock();
      return;
    }
    if (line_len && line[line_len - 1] == '\n')
      line_len--;
    line_cluster_len = count_clusters(line, line_len, 0, line_len);
    uint32_t target_row = next_unfolded_row(editor, editor->cursor.row + 1);
    if (target_row >= editor->root->line_count) {
      free(line);
      lock.unlock();
      return;
    }
    uint32_t down_by = target_row - editor->cursor.row;
    if (down_by > 1)
      down_by--;
    uint32_t ln;
    line_to_byte(editor->root, editor->cursor.row + down_by - 1, &ln);
    lock.unlock();
    Coord cursor = editor->cursor;
    edit_erase(editor, {cursor.row, 0}, line_cluster_len);
    edit_erase(editor, {cursor.row, 0}, -1);
    edit_insert(editor, {cursor.row + down_by, 0}, (char *)"\n", 1);
    edit_insert(editor, {cursor.row + down_by, 0}, line, line_len);
    free(it->buffer);
    free(it);
    editor->cursor = {cursor.row + down_by, cursor.col};
  } else if (mode == SELECT) {
    if (editor->cursor.row >= editor->root->line_count - 1 ||
        editor->selection.row >= editor->root->line_count - 1)
      return;
    std::shared_lock lock(editor->knot_mtx);
    uint32_t start_row = MIN(editor->cursor.row, editor->selection.row);
    uint32_t end_row = MAX(editor->cursor.row, editor->selection.row);
    uint32_t target_row = next_unfolded_row(editor, end_row + 1);
    if (target_row >= editor->root->line_count)
      return;
    uint32_t down_by = target_row - end_row;
    if (down_by > 1)
      down_by--;
    uint32_t start_byte = line_to_byte(editor->root, start_row, nullptr);
    uint32_t end_byte = line_to_byte(editor->root, end_row + 1, nullptr);
    char *selected_text = read(editor->root, start_byte, end_byte - start_byte);
    lock.unlock();
    if (!selected_text)
      return;
    uint32_t selected_len = count_clusters(selected_text, end_byte - start_byte,
                                           0, end_byte - start_byte);
    Coord cursor = editor->cursor;
    Coord selection = editor->selection;
    edit_erase(editor, {start_row, 0}, selected_len);
    edit_insert(editor, {start_row + down_by, 0}, selected_text,
                end_byte - start_byte);
    free(selected_text);
    editor->cursor = {cursor.row + down_by, cursor.col};
    editor->selection = {selection.row + down_by, selection.col};
  }
}

void edit_erase(Editor *editor, Coord pos, int64_t len) {
  if (len == 0)
    return;
  if (len < 0) {
    std::shared_lock lock_1(editor->knot_mtx);
    uint32_t cursor_original =
        line_to_byte(editor->root, editor->cursor.row, nullptr) +
        editor->cursor.col;
    TSPoint old_point = {pos.row, pos.col};
    uint32_t byte_pos = line_to_byte(editor->root, pos.row, nullptr) + pos.col;
    Coord point = move_left_pure(editor, pos, -len);
    json lsp_range;
    bool do_lsp = (editor->lsp != nullptr);
    if (do_lsp) {
      LineIterator *it = begin_l_iter(editor->root, point.row);
      char *line = next_line(it, nullptr);
      int utf16_start = 0;
      if (line)
        utf16_start = utf8_byte_offset_to_utf16(line, point.col);
      free(it->buffer);
      free(it);
      it = begin_l_iter(editor->root, pos.row);
      line = next_line(it, nullptr);
      int utf16_end = 0;
      if (line)
        utf16_end = utf8_byte_offset_to_utf16(line, pos.col);
      free(it->buffer);
      free(it);
      lsp_range = {{"start", {{"line", point.row}, {"character", utf16_start}}},
                   {"end", {{"line", pos.row}, {"character", utf16_end}}}};
    }
    uint32_t start = line_to_byte(editor->root, point.row, nullptr) + point.col;
    if (cursor_original > start && cursor_original <= byte_pos) {
      editor->cursor = point;
      editor->cursor_preffered = UINT32_MAX;
    } else if (cursor_original > byte_pos) {
      uint32_t cursor_new = cursor_original - (byte_pos - start);
      uint32_t new_col;
      uint32_t new_row = byte_to_line(editor->root, cursor_new, &new_col);
      editor->cursor = {new_row, new_col};
      editor->cursor_preffered = UINT32_MAX;
    }
    lock_1.unlock();
    uint32_t start_row = point.row;
    uint32_t end_row = pos.row;
    apply_line_deletion(editor, start_row + 1, end_row);
    apply_hook_deletion(editor, start_row + 1, end_row);
    std::unique_lock lock_2(editor->knot_mtx);
    editor->root = erase(editor->root, start, byte_pos - start);
    lock_2.unlock();
    if (editor->ts.tree) {
      TSInputEdit edit = {
          .start_byte = start,
          .old_end_byte = byte_pos,
          .new_end_byte = start,
          .start_point = {point.row, point.col},
          .old_end_point = old_point,
          .new_end_point = {point.row, point.col},
      };
      editor->edit_queue.push(edit);
    }
    if (do_lsp) {
      json message = {
          {"jsonrpc", "2.0"},
          {"method", "textDocument/didChange"},
          {"params",
           {{"textDocument",
             {{"uri", editor->uri}, {"version", ++editor->lsp_version}}},
            {"contentChanges",
             json::array({{{"range", lsp_range}, {"text", ""}}})}}}};
      lsp_send(editor->lsp, message, nullptr);
    }
    std::unique_lock lock_3(editor->spans.mtx);
    apply_edit(editor->spans.spans, start, start - byte_pos);
    if (editor->spans.mid_parse)
      editor->spans.edits.push({start, start - byte_pos});
    std::unique_lock lock_4(editor->def_spans.mtx);
    apply_edit(editor->def_spans.spans, byte_pos, start - byte_pos);
  } else {
    std::shared_lock lock_1(editor->knot_mtx);
    uint32_t cursor_original =
        line_to_byte(editor->root, editor->cursor.row, nullptr) +
        editor->cursor.col;
    TSPoint old_point = {pos.row, pos.col};
    uint32_t byte_pos = line_to_byte(editor->root, pos.row, nullptr) + pos.col;
    Coord point = move_right_pure(editor, pos, len);
    json lsp_range;
    bool do_lsp = (editor->lsp != nullptr);
    if (do_lsp) {
      LineIterator *it = begin_l_iter(editor->root, pos.row);
      char *line = next_line(it, nullptr);
      int utf16_start = 0;
      if (line)
        utf16_start = utf8_byte_offset_to_utf16(line, pos.col);
      free(it->buffer);
      free(it);
      it = begin_l_iter(editor->root, point.row);
      line = next_line(it, nullptr);
      int utf16_end = 0;
      if (line)
        utf16_end = utf8_byte_offset_to_utf16(line, point.col);
      free(it->buffer);
      free(it);
      lsp_range = {{"start", {{"line", pos.row}, {"character", utf16_start}}},
                   {"end", {{"line", point.row}, {"character", utf16_end}}}};
    }
    uint32_t end = line_to_byte(editor->root, point.row, nullptr) + point.col;
    if (cursor_original > byte_pos && cursor_original <= end) {
      editor->cursor = pos;
      editor->cursor_preffered = UINT32_MAX;
    } else if (cursor_original > end) {
      uint32_t cursor_new = cursor_original - (end - byte_pos);
      uint32_t new_col;
      uint32_t new_row = byte_to_line(editor->root, cursor_new, &new_col);
      editor->cursor = {new_row, new_col};
      editor->cursor_preffered = UINT32_MAX;
    }
    lock_1.unlock();
    uint32_t start_row = pos.row;
    uint32_t end_row = point.row;
    apply_line_deletion(editor, start_row + 1, end_row);
    apply_hook_deletion(editor, start_row + 1, end_row);
    std::unique_lock lock_2(editor->knot_mtx);
    editor->root = erase(editor->root, byte_pos, end - byte_pos);
    lock_2.unlock();
    if (editor->ts.tree) {
      TSInputEdit edit = {
          .start_byte = byte_pos,
          .old_end_byte = end,
          .new_end_byte = byte_pos,
          .start_point = old_point,
          .old_end_point = {point.row, point.col},
          .new_end_point = old_point,
      };
      editor->edit_queue.push(edit);
    }
    if (do_lsp) {
      json message = {
          {"jsonrpc", "2.0"},
          {"method", "textDocument/didChange"},
          {"params",
           {{"textDocument",
             {{"uri", editor->uri}, {"version", ++editor->lsp_version}}},
            {"contentChanges",
             json::array({{{"range", lsp_range}, {"text", ""}}})}}}};
      lsp_send(editor->lsp, message, nullptr);
    }
    std::unique_lock lock_3(editor->spans.mtx);
    apply_edit(editor->spans.spans, byte_pos, byte_pos - end);
    if (editor->spans.mid_parse)
      editor->spans.edits.push({byte_pos, byte_pos - end});
    std::unique_lock lock_4(editor->def_spans.mtx);
    apply_edit(editor->def_spans.spans, byte_pos, byte_pos - end);
  }
}

void edit_insert(Editor *editor, Coord pos, char *data, uint32_t len) {
  std::shared_lock lock_1(editor->knot_mtx);
  uint32_t cursor_original =
      line_to_byte(editor->root, editor->cursor.row, nullptr) +
      editor->cursor.col;
  uint32_t byte_pos = line_to_byte(editor->root, pos.row, nullptr) + pos.col;
  TSPoint start_point = {pos.row, pos.col};
  if (cursor_original > byte_pos) {
    uint32_t cursor_new = cursor_original + len;
    uint32_t new_col;
    uint32_t new_row = byte_to_line(editor->root, cursor_new, &new_col);
    editor->cursor = {new_row, new_col};
  }
  lock_1.unlock();
  std::unique_lock lock_2(editor->knot_mtx);
  editor->root = insert(editor->root, byte_pos, data, len);
  lock_2.unlock();
  uint32_t cols = 0;
  uint32_t rows = 0;
  for (uint32_t i = 0; i < len; i++) {
    if (data[i] == '\n') {
      rows++;
      cols = 0;
    } else {
      cols++;
    }
  }
  apply_line_insertion(editor, pos.row, rows);
  apply_hook_insertion(editor, pos.row, rows);
  if (editor->ts.tree) {
    TSInputEdit edit = {
        .start_byte = byte_pos,
        .old_end_byte = byte_pos,
        .new_end_byte = byte_pos + len,
        .start_point = start_point,
        .old_end_point = start_point,
        .new_end_point = {start_point.row + rows,
                          (rows == 0) ? (start_point.column + cols) : cols},
    };
    editor->edit_queue.push(edit);
  }
  if (editor->lsp) {
    lock_1.lock();
    LineIterator *it = begin_l_iter(editor->root, pos.row);
    char *line = next_line(it, nullptr);
    int utf16_col = 0;
    if (line)
      utf16_col = utf8_byte_offset_to_utf16(line, pos.col);
    free(it->buffer);
    free(it);
    lock_1.unlock();
    json message = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params",
         {{"textDocument",
           {{"uri", editor->uri}, {"version", ++editor->lsp_version}}},
          {"contentChanges",
           json::array(
               {{{"range",
                  {{"start", {{"line", pos.row}, {"character", utf16_col}}},
                   {"end", {{"line", pos.row}, {"character", utf16_col}}}}},
                 {"text", std::string(data, len)}}})}}}};
    lsp_send(editor->lsp, message, nullptr);
  }
  std::unique_lock lock_3(editor->spans.mtx);
  apply_edit(editor->spans.spans, byte_pos, len);
  if (editor->spans.mid_parse)
    editor->spans.edits.push({byte_pos, len});
  std::unique_lock lock_4(editor->def_spans.mtx);
  apply_edit(editor->def_spans.spans, byte_pos, len);
}

char *get_selection(Editor *editor, uint32_t *out_len, Coord *out_start) {
  std::shared_lock lock(editor->knot_mtx);
  Coord start, end;
  if (editor->cursor >= editor->selection) {
    uint32_t prev_col, next_col;
    switch (editor->selection_type) {
    case CHAR:
      start = editor->selection;
      end = move_right(editor, editor->cursor, 1);
      break;
    case WORD:
      word_boundaries(editor, editor->selection, &prev_col, &next_col, nullptr,
                      nullptr);
      start = {editor->selection.row, prev_col};
      end = editor->cursor;
      break;
    case LINE:
      start = {editor->selection.row, 0};
      end = editor->cursor;
      break;
    }
  } else {
    start = editor->cursor;
    uint32_t prev_col, next_col, line_len;
    switch (editor->selection_type) {
    case CHAR:
      end = move_right(editor, editor->selection, 1);
      break;
    case WORD:
      word_boundaries(editor, editor->selection, &prev_col, &next_col, nullptr,
                      nullptr);
      end = {editor->selection.row, next_col};
      break;
    case LINE:
      LineIterator *it = begin_l_iter(editor->root, editor->selection.row);
      char *line = next_line(it, &line_len);
      if (!line)
        return nullptr;
      if (line_len > 0 && line[line_len - 1] == '\n')
        line_len--;
      end = {editor->selection.row, line_len};
      free(it->buffer);
      free(it);
      break;
    }
  }
  if (out_start)
    *out_start = start;
  uint32_t start_byte =
      line_to_byte(editor->root, start.row, nullptr) + start.col;
  uint32_t end_byte = line_to_byte(editor->root, end.row, nullptr) + end.col;
  char *text = read(editor->root, start_byte, end_byte - start_byte);
  if (out_len)
    *out_len = end_byte - start_byte;
  return text;
}

void apply_edit(std::vector<Span> &spans, uint32_t x, int64_t y) {
  Span key{.start = x, .end = 0, .hl = nullptr};
  auto it = std::lower_bound(
      spans.begin(), spans.end(), key,
      [](const Span &a, const Span &b) { return a.start < b.start; });
  size_t idx = std::distance(spans.begin(), it);
  while (idx > 0 && spans.at(idx - 1).end >= x)
    --idx;
  for (size_t i = idx; i < spans.size();) {
    Span &s = spans.at(i);
    if (s.start < x && s.end >= x) {
      s.end += y;
    } else if (s.start > x) {
      s.start += y;
      s.end += y;
    }
    if (s.end <= s.start)
      spans.erase(spans.begin() + i);
    else
      ++i;
  }
}

std::vector<Fold>::iterator find_fold_iter(Editor *editor, uint32_t line) {
  auto &folds = editor->folds;
  auto it = std::lower_bound(
      folds.begin(), folds.end(), line,
      [](const Fold &fold, uint32_t value) { return fold.start < value; });
  if (it != folds.end() && it->start == line)
    return it;
  if (it != folds.begin()) {
    --it;
    if (it->contains(line))
      return it;
  }
  return folds.end();
}

bool add_fold(Editor *editor, uint32_t start, uint32_t end) {
  if (!editor || !editor->root)
    return false;
  if (start > end)
    std::swap(start, end);
  if (start >= editor->root->line_count)
    return false;
  end = std::min(end, editor->root->line_count - 1);
  if (start == end)
    return false;
  Fold new_fold{start, end};
  auto &folds = editor->folds;
  auto it = std::lower_bound(
      folds.begin(), folds.end(), new_fold.start,
      [](const Fold &fold, uint32_t value) { return fold.start < value; });
  if (it != folds.begin()) {
    auto prev = std::prev(it);
    if (prev->end + 1 >= new_fold.start) {
      new_fold.start = std::min(new_fold.start, prev->start);
      new_fold.end = std::max(new_fold.end, prev->end);
      it = folds.erase(prev);
    }
  }
  while (it != folds.end() && it->start <= new_fold.end + 1) {
    new_fold.end = std::max(new_fold.end, it->end);
    it = folds.erase(it);
  }
  folds.insert(it, new_fold);
  return true;
}

bool remove_fold(Editor *editor, uint32_t line) {
  auto it = find_fold_iter(editor, line);
  if (it == editor->folds.end())
    return false;
  editor->folds.erase(it);
  return true;
}

void apply_line_insertion(Editor *editor, uint32_t line, uint32_t rows) {
  for (auto it = editor->folds.begin(); it != editor->folds.end();) {
    if (line <= it->start) {
      it->start += rows;
      it->end += rows;
      ++it;
    } else if (line <= it->end) {
      it = editor->folds.erase(it);
    } else {
      ++it;
    }
  }
}

void apply_line_deletion(Editor *editor, uint32_t removal_start,
                         uint32_t removal_end) {
  if (removal_start > removal_end)
    return;
  uint32_t rows_removed = removal_end - removal_start + 1;
  std::vector<Fold> updated;
  updated.reserve(editor->folds.size());
  for (auto fold : editor->folds) {
    if (removal_end < fold.start) {
      fold.start -= rows_removed;
      fold.end -= rows_removed;
      updated.push_back(fold);
      continue;
    }
    if (removal_start > fold.end) {
      updated.push_back(fold);
      continue;
    }
  }
  editor->folds.swap(updated);
}

void apply_hook_insertion(Editor *editor, uint32_t line, uint32_t rows) {
  for (auto &hook : editor->hooks)
    if (hook > line)
      hook += rows;
}

void apply_hook_deletion(Editor *editor, uint32_t removal_start,
                         uint32_t removal_end) {
  for (auto &hook : editor->hooks)
    if (hook > removal_start)
      hook -= removal_end - removal_start + 1;
}

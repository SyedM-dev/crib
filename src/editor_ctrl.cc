extern "C" {
#include "../libs/libgrapheme/grapheme.h"
}
#include "../include/editor.h"
#include "../include/main.h"
#include "../include/ts.h"
#include "../include/utils.h"
#include <cmath>

static Highlight HL_UNDERLINE = {0, 0, 1 << 2, 100};

static std::vector<Fold>::iterator find_fold_iter(Editor *editor,
                                                  uint32_t line) {
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

static bool add_fold(Editor *editor, uint32_t start, uint32_t end) {
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

static bool remove_fold(Editor *editor, uint32_t line) {
  auto it = find_fold_iter(editor, line);
  if (it == editor->folds.end())
    return false;
  editor->folds.erase(it);
  return true;
}

static void apply_line_insertion(Editor *editor, uint32_t line,
                                 uint32_t rows) {
  if (rows == 0)
    return;
  for (auto &fold : editor->folds) {
    if (line <= fold.start) {
      fold.start += rows;
      fold.end += rows;
    } else if (line <= fold.end) {
      fold.end += rows;
    }
  }
}

static void apply_line_deletion(Editor *editor, uint32_t removal_start,
                                uint32_t removal_end) {
  if (removal_start > removal_end)
    return;
  uint32_t rows_removed = removal_end - removal_start + 1;
  std::vector<Fold> updated;
  updated.reserve(editor->folds.size());
  for (auto fold : editor->folds) {
    if (fold.end < removal_start) {
      updated.push_back(fold);
      continue;
    }
    if (fold.start > removal_end) {
      fold.start -= rows_removed;
      fold.end -= rows_removed;
      updated.push_back(fold);
      continue;
    }
    if (fold.start >= removal_start)
      continue;
    if (fold.end > removal_end)
      fold.end -= rows_removed;
    else
      fold.end = removal_start - 1;
    if (fold.end >= fold.start)
      updated.push_back(fold);
  }
  editor->folds.swap(updated);
}

void handle_editor_event(Editor *editor, KeyEvent event) {
  static std::chrono::steady_clock::time_point last_click_time =
      std::chrono::steady_clock::now();
  static uint32_t click_count = 0;
  static Coord last_click_pos = {UINT32_MAX, UINT32_MAX};
  if (event.key_type == KEY_MOUSE) {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - last_click_time)
                        .count();
    switch (event.mouse_state) {
    case SCROLL:
      switch (event.mouse_direction) {
      case SCROLL_UP:
        scroll_up(editor, 10);
        ensure_cursor(editor);
        break;
      case SCROLL_DOWN:
        scroll_down(editor, 10);
        ensure_cursor(editor);
        break;
      case SCROLL_LEFT:
        cursor_left(editor, 10);
        break;
      case SCROLL_RIGHT:
        cursor_right(editor, 10);
        break;
      }
      break;
    case PRESS:
      if (event.mouse_button == LEFT_BTN) {
        Coord cur_pos = {event.mouse_x, event.mouse_y};
        if (duration < 250 && last_click_pos == cur_pos)
          click_count++;
        else
          click_count = 1;
        last_click_time = now;
        last_click_pos = cur_pos;
        Coord p = editor_hit_test(editor, event.mouse_x, event.mouse_y);
        if (p.row == UINT32_MAX && p.col == UINT32_MAX)
          return;
        editor->cursor_preffered = UINT32_MAX;
        if (click_count == 1) {
          editor->cursor = p;
          editor->selection = p;
          if (mode == SELECT) {
            mode = NORMAL;
            editor->selection_active = false;
          }
        } else if (click_count == 2) {
          uint32_t prev_col, next_col;
          word_boundaries(editor, editor->cursor, &prev_col, &next_col, nullptr,
                          nullptr);
          if (editor->cursor < editor->selection)
            editor->cursor = {editor->cursor.row, prev_col};
          else
            editor->cursor = {editor->cursor.row, next_col};
          editor->cursor_preffered = UINT32_MAX;
          editor->selection_type = WORD;
          mode = SELECT;
          editor->selection_active = true;
        } else if (click_count >= 3) {
          if (editor->cursor < editor->selection) {
            editor->cursor = {p.row, 0};
          } else {
            uint32_t line_len;
            LineIterator *it = begin_l_iter(editor->root, p.row);
            char *line = next_line(it, &line_len);
            free(it);
            if (!line)
              return;
            if (line_len > 0 && line[line_len - 1] == '\n')
              line_len--;
            free(line);
            editor->cursor = {p.row, line_len};
          }
          editor->cursor_preffered = UINT32_MAX;
          editor->selection_type = LINE;
          mode = SELECT;
          editor->selection_active = true;
          click_count = 3;
        }
      }
      break;
    case DRAG:
      if (event.mouse_button == LEFT_BTN) {
        Coord p = editor_hit_test(editor, event.mouse_x, event.mouse_y);
        if (p.row == UINT32_MAX && p.col == UINT32_MAX)
          return;
        editor->cursor_preffered = UINT32_MAX;
        mode = SELECT;
        if (!editor->selection_active) {
          editor->selection_active = true;
          editor->selection_type = CHAR;
        }
        uint32_t prev_col, next_col, line_len;
        switch (editor->selection_type) {
        case CHAR:
          editor->cursor = p;
          break;
        case WORD:
          word_boundaries(editor, p, &prev_col, &next_col, nullptr, nullptr);
          if (editor->cursor < editor->selection)
            editor->cursor = {p.row, prev_col};
          else
            editor->cursor = {p.row, next_col};
          break;
        case LINE:
          if (editor->cursor < editor->selection) {
            editor->cursor = {p.row, 0};
          } else {
            LineIterator *it = begin_l_iter(editor->root, p.row);
            char *line = next_line(it, &line_len);
            free(it);
            if (!line)
              return;
            if (line_len > 0 && line[line_len - 1] == '\n')
              line_len--;
            free(line);
            editor->cursor = {p.row, line_len};
          }
          break;
        }
      }
      break;
    case RELEASE:
      if (event.mouse_button == LEFT_BTN)
        if (editor->cursor.row == editor->selection.row &&
            editor->cursor.col == editor->selection.col) {
          mode = NORMAL;
          editor->selection_active = false;
        }
      break;
    }
  }
  if (event.key_type == KEY_SPECIAL) {
    switch (event.special_modifier) {
    case 0:
      switch (event.special_key) {
      case KEY_DOWN:
        cursor_down(editor, 1);
        break;
      case KEY_UP:
        cursor_up(editor, 1);
        break;
      case KEY_LEFT:
        cursor_left(editor, 1);
        break;
      case KEY_RIGHT:
        cursor_right(editor, 1);
        break;
      }
      break;
    case CNTRL:
      uint32_t prev_col, next_col;
      word_boundaries(editor, editor->cursor, &prev_col, &next_col, nullptr,
                      nullptr);
      switch (event.special_key) {
      case KEY_DOWN:
        cursor_down(editor, 5);
        break;
      case KEY_UP:
        cursor_up(editor, 5);
        break;
      case KEY_LEFT:
        editor->cursor_preffered = UINT32_MAX;
        if (prev_col == editor->cursor.col)
          cursor_left(editor, 1);
        else
          editor->cursor = {editor->cursor.row, prev_col};
        break;
      case KEY_RIGHT:
        editor->cursor_preffered = UINT32_MAX;
        if (next_col == editor->cursor.col)
          cursor_right(editor, 1);
        else
          editor->cursor = {editor->cursor.row, next_col};
        break;
      }
      break;
    case ALT:
      switch (event.special_key) {
      case KEY_DOWN:
        move_line_down(editor);
        break;
      case KEY_UP:
        move_line_up(editor);
        break;
      case KEY_LEFT:
        cursor_left(editor, 8);
        break;
      case KEY_RIGHT:
        cursor_right(editor, 8);
        break;
      }
      break;
    }
  }
  switch (mode) {
  case NORMAL:
    if (event.key_type == KEY_CHAR && event.len == 1) {
      switch (event.c[0]) {
      case 'u':
        if (editor->root->line_count > 0) {
          editor->cursor.row = editor->root->line_count - 1;
          LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
          if (!it)
            break;
          uint32_t line_len;
          char *line = next_line(it, &line_len);
          if (!line)
            break;
          if (line_len > 0 && line[line_len - 1] == '\n')
            line_len--;
          free(line);
          free(it);
          line_len = count_clusters(line, line_len, 0, line_len);
          editor->cursor.col = line_len;
          editor->cursor_preffered = UINT32_MAX;
          mode = SELECT;
          editor->selection_active = true;
          editor->selection = {0, 0};
          editor->selection_type = LINE;
        }
        break;
      case 'a':
        mode = INSERT;
        cursor_right(editor, 1);
        break;
      case 'i':
        mode = INSERT;
        break;
      case 'n':
        mode = JUMPER;
        editor->jumper_set = true;
        break;
      case 'm':
        mode = JUMPER;
        editor->jumper_set = false;
        break;
      case 'N':
        for (uint8_t i = 0; i < 94; i++)
          if (editor->hooks[i] == editor->cursor.row + 1) {
            editor->hooks[i] = 0;
            break;
          }
        break;
      case 's':
      case 'v':
        mode = SELECT;
        editor->selection_active = true;
        editor->selection = editor->cursor;
        editor->selection_type = CHAR;
        break;
      case ';':
      case ':':
        mode = RUNNER;
        break;
      case 0x7F:
        cursor_left(editor, 1);
        break;
      case ' ':
        cursor_right(editor, 1);
        break;
      case '\r':
      case '\n':
        cursor_down(editor, 1);
        break;
      case '\\':
      case '|':
        cursor_up(editor, 1);
        break;
      case CTRL('d'):
        scroll_down(editor, 1);
        ensure_cursor(editor);
        break;
      case CTRL('u'):
        scroll_up(editor, 1);
        ensure_cursor(editor);
        break;
      case 'p':
        uint32_t len;
        char *text = get_from_clipboard(&len);
        if (text) {
          edit_insert(editor, editor->cursor, text, len);
          uint32_t grapheme_len = count_clusters(text, len, 0, len);
          cursor_right(editor, grapheme_len);
          free(text);
        }
        break;
      }
    }
    break;
  case INSERT:
    if (event.key_type == KEY_CHAR) {
      if (event.len == 1) {
        if (event.c[0] == '\t') {
          edit_insert(editor, editor->cursor, (char *)"  ", 1);
          cursor_right(editor, 2);
        } else if (event.c[0] == '\n' || event.c[0] == '\r') {
          edit_insert(editor, editor->cursor, (char *)"\n", 1);
          cursor_right(editor, 1);
        } else if (event.c[0] == 0x7F) {
          edit_erase(editor, editor->cursor, -1);
        } else if (event.c[0] == CTRL('W')) {
          uint32_t prev_col_byte, prev_col_cluster;
          word_boundaries(editor, editor->cursor, &prev_col_byte, nullptr,
                          &prev_col_cluster, nullptr);
          if (prev_col_byte == editor->cursor.col)
            edit_erase(editor, editor->cursor, -1);
          else
            edit_erase(editor, editor->cursor, -(int64_t)prev_col_cluster);
        } else if (isprint((unsigned char)(event.c[0]))) {
          edit_insert(editor, editor->cursor, event.c, 1);
          cursor_right(editor, 1);
        } else if (event.c[0] == 0x1B) {
          mode = NORMAL;
          cursor_left(editor, 1);
        }
      } else if (event.len > 1) {
        edit_insert(editor, editor->cursor, event.c, event.len);
        cursor_right(editor, 1);
      }
    } else if (event.key_type == KEY_SPECIAL &&
               event.special_key == KEY_DELETE) {
      switch (event.special_modifier) {
      case 0:
        edit_erase(editor, editor->cursor, 1);
        break;
      case CNTRL:
        uint32_t next_col_byte, next_col_cluster;
        word_boundaries(editor, editor->cursor, nullptr, &next_col_byte,
                        nullptr, &next_col_cluster);
        if (next_col_byte == editor->cursor.col)
          edit_erase(editor, editor->cursor, 1);
        else
          edit_erase(editor, editor->cursor, next_col_cluster);
        break;
      }
    }
    break;
  case SELECT:
    if (event.key_type == KEY_CHAR && event.len == 1) {
      uint32_t len;
      char *text;
      switch (event.c[0]) {
      case 'f':
        if (editor->cursor.row != editor->selection.row) {
          uint32_t start = MIN(editor->cursor.row, editor->selection.row);
          uint32_t end = MAX(editor->cursor.row, editor->selection.row);
          add_fold(editor, start, end);
        }
        cursor_left(editor, 1);
        cursor_down(editor, 1);
        editor->selection_active = false;
        mode = NORMAL;
        break;
      case 0x1B:
      case 's':
      case 'v':
        editor->selection_active = false;
        mode = NORMAL;
        break;
      case 'y':
        text = get_selection(editor, &len);
        copy_to_clipboard(text, len);
        free(text);
        editor->selection_active = false;
        mode = NORMAL;
        break;
      case 'x':
        text = get_selection(editor, &len);
        copy_to_clipboard(text, len);
        edit_erase(editor, MIN(editor->cursor, editor->selection), len);
        free(text);
        editor->selection_active = false;
        mode = NORMAL;
        break;
      case 'p':
        text = get_from_clipboard(&len);
        if (text) {
          Coord start, end;
          if (editor->cursor >= editor->selection) {
            start = editor->selection;
            end = move_right(editor, editor->cursor, 1);
          } else {
            start = editor->cursor;
            end = move_right(editor, editor->selection, 1);
          }
          uint32_t start_byte =
              line_to_byte(editor->root, start.row, nullptr) + start.col;
          uint32_t end_byte =
              line_to_byte(editor->root, end.row, nullptr) + end.col;
          edit_erase(editor, start, end_byte - start_byte);
          edit_insert(editor, editor->cursor, text, len);
          free(text);
        }
        editor->selection_active = false;
        mode = NORMAL;
        break;
      }
    }
    break;
  case JUMPER:
    if (event.key_type == KEY_CHAR && event.len == 1 &&
        (event.c[0] >= '!' && event.c[0] <= '~')) {
      if (editor->jumper_set) {
        for (uint8_t i = 0; i < 94; i++)
          if (editor->hooks[i] == editor->cursor.row + 1) {
            editor->hooks[i] = 0;
            break;
          }
        editor->hooks[event.c[0] - '!'] = editor->cursor.row + 1;
      } else {
        uint32_t line = editor->hooks[event.c[0] - '!'];
        if (line > 0) {
          if (line_is_folded(editor->folds, --line))
            break;
          editor->cursor = {line, 0};
          editor->cursor_preffered = UINT32_MAX;
        }
      }
    }
    mode = NORMAL;
    break;
  case RUNNER:
    if (event.key_type == KEY_CHAR && event.len == 1) {
      switch (event.c[0]) {
      case 0x1B:
        mode = NORMAL;
        break;
      }
    }
    break;
  }
  ensure_scroll(editor);
  if (event.key_type == KEY_CHAR && event.c)
    free(event.c);
}

void editor_worker(Editor *editor) {
  if (!editor || !editor->root)
    return;
  if (editor->parser && editor->query)
    ts_collect_spans(editor);
  uint32_t prev_col, next_col;
  word_boundaries_exclusive(editor, editor->cursor, &prev_col, &next_col);
  if (next_col - prev_col > 0 && next_col - prev_col < 256 - 4) {
    std::shared_lock lock(editor->knot_mtx);
    uint32_t offset = line_to_byte(editor->root, editor->cursor.row, nullptr);
    char *word = read(editor->root, offset + prev_col, next_col - prev_col);
    if (word) {
      char buf[256];
      snprintf(buf, sizeof(buf), "\\b%s\\b", word);
      std::vector<std::pair<size_t, size_t>> results =
          search_rope(editor->root, buf);
      std::unique_lock lock(editor->def_spans.mtx);
      editor->def_spans.spans.clear();
      for (const auto &match : results) {
        Span s;
        s.start = match.first;
        s.end = match.first + match.second;
        s.hl = &HL_UNDERLINE;
        editor->def_spans.spans.push_back(s);
      }
      std::sort(editor->def_spans.spans.begin(), editor->def_spans.spans.end());
      lock.unlock();
      free(word);
    }
  } else {
    std::unique_lock lock(editor->def_spans.mtx);
    editor->def_spans.spans.clear();
    lock.unlock();
  }
}

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
  free(it);
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
  free(line);
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
  free(it);
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
  free(line);
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
  bool first_visual_line = true;
  std::shared_lock knot_lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, line_index);
  if (!it)
    return editor->scroll;
  while (visual_row <= target_visual_row) {
    const Fold *fold = fold_for_line(editor->folds, line_index);
    if (fold) {
      if (visual_row == target_visual_row) {
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
        free(l);
        line_index++;
      }
      continue;
    }
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line)
      break;
    if (line_len && line[line_len - 1] == '\n')
      line_len--;
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
          free(line);
          free(it);
          return {line_index, offset + advance};
        }
        advance += g;
        last_good_offset = offset + advance;
        left -= g;
        col += w;
      }
      if (visual_row == target_visual_row) {
        free(line);
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
    free(line);
    line_index++;
  }
  free(it);
  return editor->scroll;
}

Coord move_right_pure(Editor *editor, Coord cursor, uint32_t number) {
  Coord result = cursor;
  if (!editor || !editor->root || number == 0)
    return result;

  uint32_t row = result.row;
  uint32_t col = result.col;
  uint32_t line_len = 0;

  LineIterator *it = begin_l_iter(editor->root, row);
  char *line = next_line(it, &line_len);
  free(it);

  if (!line)
    return result;

  if (line_len > 0 && line[line_len - 1] == '\n')
    --line_len;

  while (number > 0) {
    if (col >= line_len) {
      free(line);
      line = nullptr;

      uint32_t next_row = row + 1;
      if (next_row >= editor->root->line_count) {
        col = line_len;
        break;
      }

      row = next_row;
      col = 0;

      it = begin_l_iter(editor->root, row);
      line = next_line(it, &line_len);
      free(it);

      if (!line)
        break;

      if (line_len > 0 && line[line_len - 1] == '\n')
        --line_len;
    } else {
      uint32_t inc =
          grapheme_next_character_break_utf8(line + col, line_len - col);
      if (inc == 0)
        break;
      col += inc;
    }
    number--;
  }

  if (line)
    free(line);

  result.row = row;
  result.col = col;
  return result;
}

Coord move_left_pure(Editor *editor, Coord cursor, uint32_t number) {
  Coord result = cursor;
  if (!editor || !editor->root || number == 0)
    return result;
  uint32_t row = result.row;
  uint32_t col = result.col;
  uint32_t len = 0;
  LineIterator *it = begin_l_iter(editor->root, row);
  char *line = next_line(it, &len);
  if (!line) {
    free(it);
    return result;
  }
  if (len > 0 && line[len - 1] == '\n')
    --len;
  bool iterator_ahead = true;
  while (number > 0) {
    if (col == 0) {
      if (row == 0)
        break;
      if (iterator_ahead) {
        free(prev_line(it, nullptr));
        iterator_ahead = false;
      }
      free(line);
      line = nullptr;
      row--;
      line = prev_line(it, &len);
      if (!line)
        break;
      if (len > 0 && line[len - 1] == '\n')
        --len;
      col = len;
    } else {
      uint32_t new_col = 0;
      while (new_col < col) {
        uint32_t inc =
            grapheme_next_character_break_utf8(line + new_col, len - new_col);
        if (new_col + inc >= col)
          break;
        new_col += inc;
      }
      col = new_col;
    }
    number--;
  }
  if (line)
    free(line);
  free(it);
  result.row = row;
  result.col = col;
  return result;
}

Coord move_right(Editor *editor, Coord cursor, uint32_t number) {
  Coord result = cursor;
  if (!editor || !editor->root || number == 0)
    return result;
  uint32_t row = result.row;
  uint32_t col = result.col;
  uint32_t line_len = 0;
  LineIterator *it = begin_l_iter(editor->root, row);
  char *line = next_line(it, &line_len);
  free(it);
  if (!line)
    return result;
  if (line_len > 0 && line[line_len - 1] == '\n')
    --line_len;
  while (number > 0) {
    if (col >= line_len) {
      free(line);
      line = nullptr;
      uint32_t next_row = next_unfolded_row(editor, row + 1);
      if (next_row >= editor->root->line_count) {
        col = line_len;
        break;
      }
      row = next_row;
      col = 0;
      it = begin_l_iter(editor->root, row);
      line = next_line(it, &line_len);
      free(it);
      if (!line)
        break;
      if (line_len > 0 && line[line_len - 1] == '\n')
        --line_len;
    } else {
      uint32_t inc =
          grapheme_next_character_break_utf8(line + col, line_len - col);
      if (inc == 0)
        break;
      col += inc;
    }
    number--;
  }
  if (line)
    free(line);
  result.row = row;
  result.col = col;
  return result;
}

Coord move_left(Editor *editor, Coord cursor, uint32_t number) {
  Coord result = cursor;
  if (!editor || !editor->root || number == 0)
    return result;
  uint32_t row = result.row;
  uint32_t col = result.col;
  uint32_t len = 0;
  LineIterator *it = begin_l_iter(editor->root, row);
  char *line = next_line(it, &len);
  if (!line) {
    free(it);
    return result;
  }
  if (len > 0 && line[len - 1] == '\n')
    --len;
  bool iterator_ahead = true;
  while (number > 0) {
    if (col == 0) {
      if (row == 0)
        break;
      if (iterator_ahead) {
        free(prev_line(it, nullptr));
        iterator_ahead = false;
      }
      free(line);
      line = nullptr;
      while (row > 0) {
        row--;
        line = prev_line(it, &len);
        if (!line)
          break;
        const Fold *fold = fold_for_line(editor->folds, row);
        if (fold) {
          while (line && row > fold->start) {
            free(line);
            line = prev_line(it, &len);
            row--;
          }
          if (line) {
            free(line);
            line = nullptr;
          }
          continue;
        }
        break;
      }
      if (!line)
        break;
      if (len > 0 && line[len - 1] == '\n')
        --len;
      col = len;
    } else {
      uint32_t new_col = 0;
      while (new_col < col) {
        uint32_t inc =
            grapheme_next_character_break_utf8(line + new_col, len - new_col);
        if (new_col + inc >= col)
          break;
        new_col += inc;
      }
      col = new_col;
    }
    number--;
  }
  if (line)
    free(line);
  free(it);
  result.row = row;
  result.col = col;
  return result;
}

void cursor_down(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  uint32_t len;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
  char *line_content = next_line(it, &len);
  free(it);
  if (line_content == nullptr)
    return;
  if (editor->cursor_preffered == UINT32_MAX)
    editor->cursor_preffered =
        get_visual_col_from_bytes(line_content, len, editor->cursor.col);
  uint32_t visual_col = editor->cursor_preffered;
  free(line_content);

  uint32_t target_row = editor->cursor.row;
  while (number > 0 && target_row < editor->root->line_count - 1) {
    target_row = next_unfolded_row(editor, target_row + 1);
    if (target_row >= editor->root->line_count) {
      target_row = editor->root->line_count - 1;
      break;
    }
    number--;
  }

  it = begin_l_iter(editor->root, target_row);
  line_content = next_line(it, &len);
  free(it);
  if (!line_content)
    return;
  if (len > 0 && line_content[len - 1] == '\n')
    --len;
  editor->cursor.row = target_row;
  editor->cursor.col = get_bytes_from_visual_col(line_content, len, visual_col);
  free(line_content);
}

void cursor_up(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0 || editor->cursor.row == 0)
    return;
  uint32_t len;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
  char *line_content = next_line(it, &len);
  free(it);
  if (!line_content)
    return;
  if (editor->cursor_preffered == UINT32_MAX)
    editor->cursor_preffered =
        get_visual_col_from_bytes(line_content, len, editor->cursor.col);
  uint32_t visual_col = editor->cursor_preffered;
  free(line_content);

  uint32_t target_row = editor->cursor.row;
  while (number > 0 && target_row > 0) {
    target_row = prev_unfolded_row(editor, target_row - 1);
    if (target_row == 0) {
      number--;
      break;
    }
    number--;
  }

  it = begin_l_iter(editor->root, target_row);
  line_content = next_line(it, &len);
  free(it);
  if (line_content) {
    if (len > 0 && line_content[len - 1] == '\n')
      --len;
    editor->cursor.row = target_row;
    editor->cursor.col =
        get_bytes_from_visual_col(line_content, len, visual_col);
    free(line_content);
  } else {
    editor->cursor.row = 0;
    editor->cursor.col = 0;
  }
}

void cursor_right(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  editor->cursor = move_right(editor, editor->cursor, number);
  editor->cursor_preffered = UINT32_MAX;
}

void cursor_left(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  editor->cursor = move_left(editor, editor->cursor, number);
  editor->cursor_preffered = UINT32_MAX;
}

void move_line_up(Editor *editor) {
  if (!editor || !editor->root || editor->cursor.row == 0)
    return;
  if (mode == NORMAL || mode == INSERT) {
    uint32_t line_len, line_cluster_len;
    std::shared_lock lock(editor->knot_mtx);
    LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
    char *line = next_line(it, &line_len);
    free(it);
    if (!line) {
      lock.unlock();
      return;
    }
    if (line_len > 0 && line[line_len - 1] == '\n')
      line_len--;
    line_cluster_len = count_clusters(line, line_len, 0, line_len);
    uint32_t target_row = prev_unfolded_row(editor, editor->cursor.row - 1);
    uint32_t up_by = editor->cursor.row - target_row;
    lock.unlock();
    Coord cursor = editor->cursor;
    edit_erase(editor, {cursor.row, 0}, line_cluster_len);
    edit_erase(editor, {cursor.row, 0}, -1);
    edit_insert(editor, {cursor.row - up_by, 0}, (char *)"\n", 1);
    edit_insert(editor, {cursor.row - up_by, 0}, line, line_len);
    free(line);
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
    free(it);
    if (!line) {
      lock.unlock();
      return;
    }
    if (line_len && line[line_len - 1] == '\n')
      line_len--;
    line_cluster_len = count_clusters(line, line_len, 0, line_len);
    uint32_t target_row =
        next_unfolded_row(editor, editor->cursor.row + 1);
    if (target_row >= editor->root->line_count) {
      free(line);
      lock.unlock();
      return;
    }
    uint32_t down_by = target_row - editor->cursor.row;
    uint32_t ln;
    line_to_byte(editor->root, editor->cursor.row + down_by - 1, &ln);
    lock.unlock();
    Coord cursor = editor->cursor;
    edit_erase(editor, {cursor.row, 0}, line_cluster_len);
    edit_erase(editor, {cursor.row, 0}, -1);
    edit_insert(editor, {cursor.row + down_by, 0}, (char *)"\n", 1);
    edit_insert(editor, {cursor.row + down_by, 0}, line, line_len);
    free(line);
    editor->cursor = {cursor.row + down_by, cursor.col};
  } else if (mode == SELECT) {
    if (editor->cursor.row >= editor->root->line_count - 1 ||
        editor->selection.row >= editor->root->line_count - 1)
      return;
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
    edit_insert(editor, {start_row + 1, 0}, selected_text,
                end_byte - start_byte);
    free(selected_text);
    editor->cursor = {cursor.row + 1, cursor.col};
    editor->selection = {selection.row + 1, selection.col};
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
    if (end_row > start_row)
      apply_line_deletion(editor, start_row + 1, end_row);
    std::unique_lock lock_2(editor->knot_mtx);
    editor->root = erase(editor->root, start, byte_pos - start);
    lock_2.unlock();
    if (editor->tree) {
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
    if (end_row > start_row)
      apply_line_deletion(editor, start_row + 1, end_row);
    std::unique_lock lock_2(editor->knot_mtx);
    editor->root = erase(editor->root, byte_pos, end - byte_pos);
    lock_2.unlock();
    if (editor->tree) {
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
  if (rows > 0)
    apply_line_insertion(editor, pos.row, rows);
  if (editor->tree) {
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
  std::unique_lock lock_3(editor->spans.mtx);
  apply_edit(editor->spans.spans, byte_pos, len);
  if (editor->spans.mid_parse)
    editor->spans.edits.push({byte_pos, len});
  std::unique_lock lock_4(editor->def_spans.mtx);
  apply_edit(editor->def_spans.spans, byte_pos, len);
}

char *get_selection(Editor *editor, uint32_t *out_len) {
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
      free(it);
      if (!line)
        return nullptr;
      if (line_len > 0 && line[line_len - 1] == '\n')
        line_len--;
      end = {editor->selection.row, line_len};
      break;
    }
  }
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

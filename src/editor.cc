extern "C" {
#include "../libs/libgrapheme/grapheme.h"
}
#include "../include/editor.h"
#include "../include/ts.h"
#include "../libs/tree-sitter-ruby/bindings/c/tree-sitter-ruby.h"
#include <cstdint>
#include <fstream>

char *load_file(const char *path, uint32_t *out_len) {
  std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
  if (!file.is_open())
    return nullptr;
  std::streamsize len = file.tellg();
  if (len < 0 || (std::uint32_t)len > 0xFFFFFFFF)
    return nullptr;
  file.seekg(0, std::ios::beg);
  char *buf = (char *)malloc(static_cast<std::uint32_t>(len));
  if (!buf)
    return nullptr;
  if (file.read(buf, len)) {
    *out_len = static_cast<uint32_t>(len);
    return buf;
  } else {
    free(buf);
    return nullptr;
  }
}

Editor *new_editor(const char *filename, Coord position, Coord size) {
  Editor *editor = new Editor();
  if (!editor)
    return nullptr;
  uint32_t len = 0;
  char *str = load_file(filename, &len);
  if (!str)
    return nullptr;
  editor->filename = filename;
  editor->position = position;
  editor->size = size;
  editor->tree = nullptr;
  editor->cursor = {0, 0};
  editor->cursor_preffered = UINT32_MAX;
  editor->selection_active = false;
  editor->selection = {0, 0};
  editor->scroll = {0, 0};
  editor->root = load(str, len, optimal_chunk_size(len));
  free(str);
  editor->folded.resize(editor->root->line_count + 2);
  std::string query = get_exe_dir() + "/../grammar/ruby.scm";
  if (len < (1024 * 64)) {
    editor->parser = ts_parser_new();
    editor->language = tree_sitter_ruby();
    ts_parser_set_language(editor->parser, editor->language);
    editor->query = load_query(query.c_str(), editor);
  }
  return editor;
}

void free_editor(Editor *editor) {
  ts_parser_delete(editor->parser);
  if (editor->tree)
    ts_tree_delete(editor->tree);
  if (editor->query)
    ts_query_delete(editor->query);
  free_rope(editor->root);
  delete editor;
}

void scroll_up(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  uint32_t count = 0;
  uint32_t visible_lines_checked = 0;
  int32_t current_check_row = editor->scroll.row;
  while (visible_lines_checked < number + 1 && current_check_row >= 0) {
    if (editor->folded[current_check_row] != 1)
      visible_lines_checked++;
    count++;
    current_check_row--;
  }
  if (current_check_row < 0)
    count = editor->scroll.row;
  LineIterator *it = begin_l_iter(editor->root, editor->scroll.row - count + 1);
  std::vector<std::pair<uint32_t, uint32_t>> stack;
  stack.reserve(count);
  uint32_t lines_iterated = 0;
  uint32_t start_row = editor->scroll.row - count + 1;
  while (lines_iterated < count) {
    char *line_content = next_line(it);
    uint32_t current_idx = start_row + lines_iterated;
    int fold_state = editor->folded[current_idx];
    if (fold_state == 2) {
      stack.push_back({1, current_idx});
    } else if (fold_state == 0) {
      uint32_t len =
          (line_content != nullptr) ? grapheme_strlen(line_content) : 0;
      stack.push_back({len, current_idx});
    }
    if (line_content)
      free(line_content);
    lines_iterated++;
  }
  uint32_t ln = 0;
  uint32_t wrap_limit = editor->size.col;
  for (int i = stack.size() - 1; i >= 0; i--) {
    uint32_t len = stack[i].first;
    uint32_t row_idx = stack[i].second;
    uint32_t segments =
        (wrap_limit > 0 && len > 0) ? (len + wrap_limit - 1) / wrap_limit : 1;
    if (len == 0)
      segments = 1;
    for (int seg = (row_idx == editor->scroll.row)
                       ? editor->scroll.col / wrap_limit
                       : segments - 1;
         seg >= 0; seg--) {
      ln++;
      if (ln == number + 1) {
        editor->scroll.row = row_idx;
        editor->scroll.col = seg * wrap_limit;
        free(it);
        return;
      }
    }
  }
  if (ln < number + 1) {
    editor->scroll.row = 0;
    editor->scroll.col = 0;
  }
  free(it);
}

void scroll_down(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  LineIterator *it = begin_l_iter(editor->root, editor->scroll.row);
  uint32_t current_row = editor->scroll.row;
  uint32_t lines_moved = 0;
  uint32_t wrap_limit = editor->size.col;
  if (wrap_limit == 0)
    wrap_limit = 1;
  while (lines_moved < number) {
    char *line_content = next_line(it);
    if (line_content == nullptr)
      break;
    int fold_state = editor->folded[current_row];
    if (fold_state == 1) {
      free(line_content);
      current_row++;
      continue;
    }
    uint32_t segments = 1;
    if (fold_state == 2) {
      segments = 1;
    } else {
      uint32_t len = grapheme_strlen(line_content);
      segments = (len > 0) ? (len + wrap_limit - 1) / wrap_limit : 1;
    }
    uint32_t start_seg = (current_row == editor->scroll.row)
                             ? (editor->scroll.col / wrap_limit)
                             : 0;
    for (uint32_t seg = start_seg; seg < segments; seg++) {
      if (current_row == editor->scroll.row && seg == start_seg)
        continue;
      lines_moved++;
      if (lines_moved == number) {
        editor->scroll.row = current_row;
        editor->scroll.col = seg * wrap_limit;
        free(line_content);
        free(it);
        return;
      }
    }
    free(line_content);
    current_row++;
  }
  free(it);
}

void cursor_down(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
  char *line_content = next_line(it);
  if (line_content == nullptr)
    return;
  if (editor->cursor_preffered == UINT32_MAX)
    editor->cursor_preffered =
        get_visual_col_from_bytes(line_content, editor->cursor.col);
  uint32_t visual_col = editor->cursor_preffered;
  do {
    free(line_content);
    line_content = next_line(it);
    editor->cursor.row += 1;
    if (editor->cursor.row >= editor->root->line_count) {
      editor->cursor.row = editor->root->line_count - 1;
      break;
    };
    if (editor->folded[editor->cursor.row] != 0)
      number++;
  } while (--number > 0);
  free(it);
  if (line_content == nullptr)
    return;
  editor->cursor.col = get_bytes_from_visual_col(line_content, visual_col);
  free(line_content);
}

void cursor_up(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
  char *line_content = next_line(it);
  if (!line_content) {
    free(it);
    return;
  }
  if (editor->cursor_preffered == UINT32_MAX)
    editor->cursor_preffered =
        get_visual_col_from_bytes(line_content, editor->cursor.col);
  uint32_t visual_col = editor->cursor_preffered;
  free(line_content);
  while (number > 0 && editor->cursor.row > 0) {
    editor->cursor.row--;
    if (editor->folded[editor->cursor.row] != 0)
      continue;
    number--;
  }
  free(it);
  it = begin_l_iter(editor->root, editor->cursor.row);
  line_content = next_line(it);
  if (!line_content) {
    free(it);
    return;
  }
  editor->cursor.col = get_bytes_from_visual_col(line_content, visual_col);
  free(line_content);
  free(it);
}

void cursor_right(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
  char *line = next_line(it);
  free(it);
  if (!line)
    return;
  uint32_t line_len = strlen(line);
  if (line[line_len - 1] == '\n')
    line[--line_len] = '\0';
  while (number > 0) {
    if (editor->cursor.col >= line_len) {
      free(line);
      line = nullptr;
      uint32_t next_row = editor->cursor.row + 1;
      while (next_row < editor->root->line_count &&
             editor->folded[next_row] != 0)
        next_row++;
      if (next_row >= editor->root->line_count) {
        editor->cursor.col = line_len;
        break;
      }
      editor->cursor.row = next_row;
      editor->cursor.col = 0;
      it = begin_l_iter(editor->root, editor->cursor.row);
      line = next_line(it);
      free(it);
      if (!line)
        break;
      line_len = strlen(line);
      if (line[line_len - 1] == '\n')
        line[--line_len] = '\0';
    } else {
      uint32_t inc = grapheme_next_character_break_utf8(
          line + editor->cursor.col, line_len - editor->cursor.col);
      if (inc == 0)
        break;
      editor->cursor.col += inc;
    }
    number--;
  }
  LineIterator *it2 = begin_l_iter(editor->root, editor->cursor.row);
  char *cur_line = next_line(it2);
  free(it2);
  if (cur_line) {
    uint32_t len2 = strlen(cur_line);
    if (len2 > 0 && cur_line[len2 - 1] == '\n')
      cur_line[--len2] = '\0';
    editor->cursor_preffered =
        get_visual_col_from_bytes(cur_line, editor->cursor.col);
    free(cur_line);
  } else {
    editor->cursor_preffered = UINT32_MAX;
  }
  if (line)
    free(line);
}

void cursor_left(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
  char *line = next_line(it);
  free(it);
  if (!line)
    return;
  uint32_t len = strlen(line);
  if (line[len - 1] == '\n')
    line[--len] = '\0';
  while (number > 0) {
    if (editor->cursor.col == 0) {
      free(line);
      line = nullptr;
      if (editor->cursor.row == 0)
        break;
      int32_t prev_row = editor->cursor.row - 1;
      while (prev_row >= 0 && editor->folded[prev_row] != 0)
        prev_row--;
      if (prev_row < 0)
        break;
      editor->cursor.row = prev_row;
      it = begin_l_iter(editor->root, editor->cursor.row);
      line = next_line(it);
      free(it);
      if (!line)
        break;
      uint32_t len = strlen(line);
      if (line[len - 1] == '\n')
        line[--len] = '\0';
      editor->cursor.col = len;
    } else {
      uint32_t col = editor->cursor.col;
      uint32_t new_col = 0;
      uint32_t visual_col = 0;
      uint32_t len = strlen(line);
      while (new_col < col) {
        uint32_t inc =
            grapheme_next_character_break_utf8(line + new_col, len - new_col);
        if (new_col + inc >= col)
          break;
        new_col += inc;
        visual_col++;
      }
      editor->cursor.col = new_col;
    }
    number--;
  }
  LineIterator *it2 = begin_l_iter(editor->root, editor->cursor.row);
  char *cur_line = next_line(it2);
  free(it2);
  if (cur_line) {
    uint32_t len2 = strlen(cur_line);
    if (len2 > 0 && cur_line[len2 - 1] == '\n')
      cur_line[--len2] = '\0';
    editor->cursor_preffered =
        get_visual_col_from_bytes(cur_line, editor->cursor.col);
    free(cur_line);
  } else {
    editor->cursor_preffered = UINT32_MAX;
  }
  if (line)
    free(line);
}

void ensure_scroll(Editor *editor) {
  LineIterator *it = begin_l_iter(editor->root, editor->scroll.row);
  uint32_t rendered_lines = 0;
  uint32_t line_index = editor->scroll.row;
  char *line_content = nullptr;
  while (rendered_lines <= editor->size.row) {
    line_content = next_line(it);
    if (!line_content)
      break;
    char *line_content_t = line_content;
    if (rendered_lines == 0)
      line_content_t = line_content + editor->scroll.col;
    uint32_t len = grapheme_strlen(line_content_t);
    free(line_content);
    uint32_t wrapped_lines = (len + editor->size.col - 1) / editor->size.col;
    rendered_lines += wrapped_lines;
    line_index++;
  }
  line_index -= 2;
  free(it);
  if (editor->cursor.row >= line_index && line_content)
    scroll_down(editor, editor->cursor.row - line_index);
  if (editor->cursor.row < editor->scroll.row)
    scroll_up(editor, editor->scroll.row - editor->cursor.row);
}

void fold(Editor *editor, uint32_t start_line, uint32_t end_line) {
  if (!editor)
    return;
  for (uint32_t i = start_line; i <= end_line && i < editor->size.row; i++)
    editor->folded[i] = 1;
  editor->folded[start_line] = 2;
}

void update_render_fold_marker(uint32_t row, uint32_t cols) {
  const char *marker = "... folded ...";
  uint32_t len = strlen(marker);
  uint32_t i = 0;
  for (; i < len && i < cols; i++)
    update(row, i, (char[2]){marker[i], 0}, 0xc6c6c6, 0, 0);
  for (; i < cols; i++)
    update(row, i, " ", 0xc6c6c6, 0, 0);
}

void apply_edit(std::vector<Span> &spans, uint32_t x, int64_t y) {
  Span key{.start = x, .end = 0, .hl = nullptr};
  auto it = std::lower_bound(
      spans.begin(), spans.end(), key,
      [](const Span &a, const Span &b) { return a.start < b.start; });
  size_t idx = std::distance(spans.begin(), it);
  while (idx > 0 && spans.at(idx - 1).end > x)
    --idx;
  for (size_t i = idx; i < spans.size();) {
    Span &s = spans.at(i);
    if (s.start < x && s.end > x) {
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

void render_editor(Editor *editor) {
  uint32_t screen_rows = editor->size.row;
  uint32_t screen_cols = editor->size.col;
  uint32_t line_index = editor->scroll.row;
  SpanCursor span_cursor(editor->spans);
  std::shared_lock knot_lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, line_index);
  if (!it)
    return;
  uint32_t rendered_rows = 0;
  uint32_t global_byte_offset = line_to_byte(editor->root, line_index, nullptr);
  span_cursor.sync(global_byte_offset);
  while (rendered_rows < screen_rows) {
    if (editor->folded[line_index]) {
      if (editor->folded[line_index] == 2) {
        update_render_fold_marker(rendered_rows, screen_cols);
        rendered_rows++;
      }
      do {
        char *line = next_line(it);
        if (!line)
          break;
        global_byte_offset += strlen(line);
        if (line[strlen(line) - 1] == '\n')
          global_byte_offset--;
        global_byte_offset++;
        free(line);
        line_index++;
      } while (line_index < editor->size.row &&
               editor->folded[line_index] == 1);
      continue;
    }
    char *line = next_line(it);
    if (!line)
      break;
    uint32_t line_len = strlen(line);
    if (line_len > 0 && line[line_len - 1] == '\n')
      line_len--;
    uint32_t current_byte_offset = 0;
    if (rendered_rows == 0 && editor->scroll.col > 0) {
      uint32_t skipped_cols = 0;
      while (skipped_cols < editor->scroll.col &&
             current_byte_offset < line_len) {
        uint32_t len = grapheme_next_character_break_utf8(
            line + current_byte_offset, line_len - current_byte_offset);
        current_byte_offset += len;
        skipped_cols++;
      }
    }
    while (current_byte_offset < line_len && rendered_rows < screen_rows) {
      uint32_t slice_byte_len = 0;
      uint32_t slice_visual_cols = 0;
      uint32_t probe_offset = current_byte_offset;
      while (slice_visual_cols < screen_cols && probe_offset < line_len) {
        uint32_t len = grapheme_next_character_break_utf8(
            line + probe_offset, line_len - probe_offset);
        slice_byte_len += len;
        probe_offset += len;
        slice_visual_cols++;
      }
      uint32_t col = 0;
      uint32_t local_render_offset = 0;
      while (local_render_offset < slice_byte_len) {
        if (line_index == editor->cursor.row &&
            editor->cursor.col == (current_byte_offset + local_render_offset))
          set_cursor(editor->position.row + rendered_rows,
                     editor->position.col + col, 1);
        uint32_t absolute_byte_pos =
            global_byte_offset + current_byte_offset + local_render_offset;
        Highlight *hl = span_cursor.get_highlight(absolute_byte_pos);
        uint32_t fg = hl ? hl->fg : 0xFFFFFF;
        uint32_t bg = hl ? hl->bg : 0;
        uint8_t fl = hl ? hl->flags : 0;
        uint32_t cluster_len = grapheme_next_character_break_utf8(
            line + current_byte_offset + local_render_offset,
            slice_byte_len - local_render_offset);
        if (cluster_len == 0)
          cluster_len = 1;
        std::string cluster(line + current_byte_offset + local_render_offset,
                            cluster_len);
        update(editor->position.row + rendered_rows, editor->position.col + col,
               cluster.c_str(), fg, bg, fl);
        local_render_offset += cluster_len;
        col++;
      }
      if (line_index == editor->cursor.row &&
          editor->cursor.col == (current_byte_offset + slice_byte_len))
        set_cursor(editor->position.row + rendered_rows,
                   editor->position.col + col, 1);
      while (col < screen_cols) {
        update(editor->position.row + rendered_rows, editor->position.col + col,
               " ", 0xFFFFFF, 0, 0);
        col++;
      }
      rendered_rows++;
      current_byte_offset += slice_byte_len;
    }
    if (line_len == 0 ||
        (current_byte_offset >= line_len && rendered_rows == 0)) {
      if (editor->cursor.row == line_index)
        set_cursor(editor->position.row + rendered_rows, editor->position.col,
                   1);
      uint32_t col = 0;
      while (col < screen_cols) {
        update(editor->position.row + rendered_rows, editor->position.col + col,
               " ", 0xFFFFFF, 0, 0);
        col++;
      }
      rendered_rows++;
    }
    global_byte_offset += line_len + 1;
    line_index++;
    free(line);
  }
  while (rendered_rows < screen_rows) {
    for (uint32_t col = 0; col < screen_cols; col++)
      update(editor->position.row + rendered_rows, editor->position.col + col,
             " ", 0xFFFFFF, 0, 0);
    rendered_rows++;
  }
  free(it);
}

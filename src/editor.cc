extern "C" {
#include "../libs/libgrapheme/grapheme.h"
}
#include "../include/editor.h"
#include "../include/main.h"
#include "../include/ts.h"
#include "../include/utils.h"
#include <cmath>

Editor *new_editor(const char *filename, Coord position, Coord size) {
  Editor *editor = new Editor();
  if (!editor)
    return nullptr;
  uint32_t len = 0;
  char *str = load_file(filename, &len);
  if (!str) {
    free_editor(editor);
    return nullptr;
  }
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
  if (len <= (1024 * 128)) {
    editor->parser = ts_parser_new();
    Language language = language_for_file(filename);
    editor->language = language.fn();
    ts_parser_set_language(editor->parser, editor->language);
    std::string query = get_exe_dir() + "/../grammar/" + language.name + ".scm";
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

void render_editor(Editor *editor) {
  uint32_t sel_start = 0, sel_end = 0;
  uint32_t numlen =
      2 + static_cast<int>(std::log10(editor->root->line_count + 1));
  uint32_t render_width = editor->size.col - numlen;
  uint32_t render_x = editor->position.col + numlen;
  std::shared_lock knot_lock(editor->knot_mtx);
  if (editor->selection_active) {
    Coord start, end;
    if (editor->cursor >= editor->selection) {
      uint32_t prev_col, next_col;
      switch (editor->selection_type) {
      case CHAR:
        start = editor->selection;
        end = move_right(editor, editor->cursor, 1);
        break;
      case WORD:
        word_boundaries(editor, editor->selection, &prev_col, &next_col,
                        nullptr, nullptr);
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
        word_boundaries(editor, editor->selection, &prev_col, &next_col,
                        nullptr, nullptr);
        end = {editor->selection.row, next_col};
        break;
      case LINE:
        LineIterator *it = begin_l_iter(editor->root, editor->selection.row);
        char *line = next_line(it, &line_len);
        free(it);
        if (!line)
          return;
        if (line_len > 0 && line[line_len - 1] == '\n')
          line_len--;
        free(line);
        end = {editor->selection.row, line_len};
        break;
      }
    }
    sel_start = line_to_byte(editor->root, start.row, nullptr) + start.col;
    sel_end = line_to_byte(editor->root, end.row, nullptr) + end.col;
  }
  Coord cursor = {UINT32_MAX, UINT32_MAX};
  uint32_t line_index = editor->scroll.row;
  SpanCursor span_cursor(editor->spans);
  SpanCursor def_span_cursor(editor->def_spans);
  LineIterator *it = begin_l_iter(editor->root, line_index);
  if (!it)
    return;
  uint32_t rendered_rows = 0;
  uint32_t global_byte_offset = line_to_byte(editor->root, line_index, nullptr);
  span_cursor.sync(global_byte_offset);
  def_span_cursor.sync(global_byte_offset);
  while (rendered_rows < editor->size.row) {
    if (editor->folded[line_index]) {
      if (editor->folded[line_index] == 2) {
        update_render_fold_marker(rendered_rows, render_width);
        rendered_rows++;
      }
      do {
        uint32_t line_len;
        char *line = next_line(it, &line_len);
        if (!line)
          break;
        global_byte_offset += line_len;
        if (line_len > 0 && line[line_len - 1] == '\n')
          global_byte_offset--;
        global_byte_offset++;
        free(line);
        line_index++;
      } while (line_index < editor->size.row &&
               editor->folded[line_index] == 1);
      continue;
    }
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line)
      break;
    if (line_len > 0 && line[line_len - 1] == '\n')
      line_len--;
    uint32_t current_byte_offset = 0;
    if (rendered_rows == 0)
      current_byte_offset += editor->scroll.col;
    while (current_byte_offset < line_len && rendered_rows < editor->size.row) {
      uint32_t color = editor->cursor.row == line_index ? 0x222222 : 0;
      if (current_byte_offset == 0 || rendered_rows == 0) {
        char buf[16];
        int len = snprintf(buf, sizeof(buf), "%*u", numlen - 1, line_index + 1);
        uint32_t num_color =
            editor->cursor.row == line_index ? 0xFFFFFF : 0x555555;
        for (int i = 0; i < len; i++)
          update(editor->position.row + rendered_rows, editor->position.col + i,
                 std::string(1, buf[i]).c_str(), num_color, 0, 0);
      } else {
        for (uint32_t i = 0; i < numlen; i++)
          update(editor->position.row + rendered_rows, editor->position.col + i,
                 " ", 0, 0, 0);
      }
      uint32_t col = 0;
      uint32_t local_render_offset = 0;
      uint32_t line_left = line_len - current_byte_offset;
      while (line_left > 0 && col < render_width) {
        if (line_index == editor->cursor.row &&
            editor->cursor.col == (current_byte_offset + local_render_offset)) {
          cursor.row = editor->position.row + rendered_rows;
          cursor.col = render_x + col;
        }
        uint32_t absolute_byte_pos =
            global_byte_offset + current_byte_offset + local_render_offset;
        Highlight *hl = span_cursor.get_highlight(absolute_byte_pos);
        Highlight *def_hl = def_span_cursor.get_highlight(absolute_byte_pos);
        uint32_t fg = hl ? hl->fg : 0xFFFFFF;
        uint32_t bg = hl ? hl->bg : 0;
        uint8_t fl = hl ? hl->flags : 0;
        if (def_hl) {
          if (def_hl->fg != 0)
            fg = def_hl->fg;
          if (def_hl->bg != 0)
            bg = def_hl->bg;
          fl |= def_hl->flags;
        }
        if (editor->selection_active && absolute_byte_pos >= sel_start &&
            absolute_byte_pos < sel_end)
          bg = 0x555555;
        uint32_t cluster_len = grapheme_next_character_break_utf8(
            line + current_byte_offset + local_render_offset, line_left);
        std::string cluster(line + current_byte_offset + local_render_offset,
                            cluster_len);
        int width = display_width(cluster.c_str(), cluster_len);
        if (col + width > render_width)
          break;
        update(editor->position.row + rendered_rows, render_x + col,
               cluster.c_str(), fg, bg | color, fl);
        local_render_offset += cluster_len;
        line_left -= cluster_len;
        col += width;
        while (width-- > 1)
          update(editor->position.row + rendered_rows, render_x + col - width,
                 "\x1b", fg, bg | color, fl);
      }
      if (line_index == editor->cursor.row &&
          editor->cursor.col == (current_byte_offset + local_render_offset)) {
        cursor.row = editor->position.row + rendered_rows;
        cursor.col = render_x + col;
      }
      if (editor->selection_active &&
          global_byte_offset + line_len + 1 > sel_start &&
          global_byte_offset + line_len + 1 <= sel_end && col < render_width) {
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               0x555555 | color, 0);
        col++;
      }
      while (col < render_width) {
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               0 | color, 0);
        col++;
      }
      rendered_rows++;
      current_byte_offset += local_render_offset;
    }
    if (line_len == 0 ||
        (current_byte_offset >= line_len && rendered_rows == 0)) {
      uint32_t color = editor->cursor.row == line_index ? 0x222222 : 0;
      char buf[16];
      int len = snprintf(buf, sizeof(buf), "%*u", numlen - 1, line_index + 1);
      uint32_t num_color =
          editor->cursor.row == line_index ? 0xFFFFFF : 0x555555;
      for (int i = 0; i < len; i++)
        update(editor->position.row + rendered_rows, editor->position.col + i,
               std::string(1, buf[i]).c_str(), num_color, 0, 0);
      if (editor->cursor.row == line_index) {
        cursor.row = editor->position.row + rendered_rows;
        cursor.col = render_x;
      }
      uint32_t col = 0;
      if (editor->selection_active &&
          global_byte_offset + line_len + 1 > sel_start &&
          global_byte_offset + line_len + 1 <= sel_end) {
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               0x555555 | color, 0);
        col++;
      }
      while (col < render_width) {
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               0 | color, 0);
        col++;
      }
      rendered_rows++;
    }
    global_byte_offset += line_len + 1;
    line_index++;
    free(line);
  }
  if (cursor.row != UINT32_MAX && cursor.col != UINT32_MAX) {
    int type = 0;
    switch (mode) {
    case NORMAL:
      type = BLOCK;
      break;
    case INSERT:
      type = CURSOR;
      break;
    case SELECT:
      type = UNDERLINE;
      break;
    }
    set_cursor(cursor.row, cursor.col, type, true);
  }
  while (rendered_rows < editor->size.row) {
    for (uint32_t col = 0; col < editor->size.col; col++)
      update(editor->position.row + rendered_rows, editor->position.col + col,
             " ", 0xFFFFFF, 0, 0);
    rendered_rows++;
  }
  free(it);
}

#include "editor/editor.h"

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

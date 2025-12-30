#include "editor/editor.h"

uint32_t leading_indent(const char *line, uint32_t len) {
  uint32_t indent = 0;
  for (uint32_t i = 0; i < len; i++) {
    if (line[i] == ' ')
      indent++;
    else if (line[i] == '\t')
      indent += INDENT_WIDTH;
    else
      break;
  }
  return indent;
}

uint32_t get_indent(Editor *editor, Coord cursor) {
  if (!editor)
    return 0;
  LineIterator *it = begin_l_iter(editor->root, cursor.row);
  next_line(it, nullptr);
  uint32_t line_len;
  char *line;
  while ((line = prev_line(it, &line_len)) != nullptr) {
    if (line_len == 0)
      continue;
    uint32_t indent = leading_indent(line, line_len);
    free(it->buffer);
    free(it);
    return indent;
  }
  free(it->buffer);
  free(it);
  return 0;
}

bool closing_after_cursor(const char *line, uint32_t len, uint32_t col) {
  uint32_t i = col;
  while (i < len && (line[i] == ' ' || line[i] == '\t'))
    i++;
  if (i >= len)
    return false;
  return line[i] == '}' || line[i] == ']' || line[i] == ')';
}

void indent_line(Editor *editor, uint32_t row) {
  if (!editor)
    return;
  LineIterator *it = begin_l_iter(editor->root, row);
  uint32_t line_len;
  char *line = next_line(it, &line_len);
  if (!line) {
    free(it->buffer);
    free(it);
    return;
  }
  char *spaces = (char *)malloc(INDENT_WIDTH);
  memset(spaces, ' ', INDENT_WIDTH);
  Coord cursor = editor->cursor;
  edit_insert(editor, {row, 0}, spaces, INDENT_WIDTH);
  editor->cursor = cursor;
  free(spaces);
  free(it->buffer);
  free(it);
}

void dedent_line(Editor *editor, uint32_t row) {
  if (!editor)
    return;
  LineIterator *it = begin_l_iter(editor->root, row);
  uint32_t line_len;
  char *line = next_line(it, &line_len);
  if (!line) {
    free(it->buffer);
    free(it);
    return;
  }
  uint32_t indent = leading_indent(line, line_len);
  if (indent == 0) {
    free(it->buffer);
    free(it);
    return;
  }
  uint32_t remove = indent >= INDENT_WIDTH ? INDENT_WIDTH : indent;
  edit_erase(editor, {row, 0}, remove);
  free(it->buffer);
  free(it);
}

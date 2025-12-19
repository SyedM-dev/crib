#include "../include/editor.h"

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
  uint32_t line_len;
  char *line;
  while ((line = prev_line(it, &line_len)) != nullptr) {
    if (line_len == 0)
      continue;
    uint32_t indent = leading_indent(line, line_len);
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

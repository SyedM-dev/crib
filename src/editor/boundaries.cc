#include "editor/editor.h"

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

void Editor::word_boundaries_exclusive(Coord coord, uint32_t *prev_col,
                                       uint32_t *next_col) {
  LineIterator *it = begin_l_iter(this->root, coord.row);
  if (!it)
    return;
  uint32_t line_len;
  char *line = next_line(it, &line_len);
  if (!line) {
    free(it->buffer);
    free(it);
    return;
  }
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

void Editor::word_boundaries(Coord coord, uint32_t *prev_col,
                             uint32_t *next_col, uint32_t *prev_clusters,
                             uint32_t *next_clusters) {
  LineIterator *it = begin_l_iter(this->root, coord.row);
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

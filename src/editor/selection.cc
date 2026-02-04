#include "editor/editor.h"
#include "utils/utils.h"

void Editor::selection_bounds(Coord *out_start, Coord *out_end) {
  Coord start, end;
  if (this->cursor >= this->selection) {
    uint32_t prev_col;
    switch (this->selection_type) {
    case CHAR:
      start = this->selection;
      end = this->move_right(this->cursor, 1);
      break;
    case WORD:
      this->word_boundaries(this->selection, &prev_col, nullptr, nullptr,
                            nullptr);
      start = {this->selection.row, prev_col};
      end = this->cursor;
      break;
    case LINE:
      start = {this->selection.row, 0};
      end = this->cursor;
      break;
    }
  } else {
    start = this->cursor;
    uint32_t next_col, line_len;
    switch (this->selection_type) {
    case CHAR:
      end = this->move_right(this->selection, 1);
      break;
    case WORD:
      this->word_boundaries(this->selection, nullptr, &next_col, nullptr,
                            nullptr);
      end = {this->selection.row, next_col};
      break;
    case LINE:
      LineIterator *it = begin_l_iter(this->root, this->selection.row);
      char *line = next_line(it, &line_len);
      if (!line)
        return;
      if (line_len > 0 && line[line_len - 1] == '\n')
        line_len--;
      end = {this->selection.row, line_len};
      free(it->buffer);
      free(it);
      break;
    }
  }
  if (out_start)
    *out_start = start;
  if (out_end)
    *out_end = end;
}

char *Editor::get_selection(uint32_t *out_len, Coord *out_start) {
  Coord start, end;
  if (this->cursor >= this->selection) {
    uint32_t prev_col;
    switch (this->selection_type) {
    case CHAR:
      start = this->selection;
      end = this->move_right(this->cursor, 1);
      break;
    case WORD:
      this->word_boundaries(this->selection, &prev_col, nullptr, nullptr,
                            nullptr);
      start = {this->selection.row, prev_col};
      end = this->cursor;
      break;
    case LINE:
      start = {this->selection.row, 0};
      end = this->cursor;
      break;
    }
  } else {
    start = this->cursor;
    uint32_t next_col, line_len;
    switch (this->selection_type) {
    case CHAR:
      end = this->move_right(this->selection, 1);
      break;
    case WORD:
      this->word_boundaries(this->selection, nullptr, &next_col, nullptr,
                            nullptr);
      end = {this->selection.row, next_col};
      break;
    case LINE:
      LineIterator *it = begin_l_iter(this->root, this->selection.row);
      char *line = next_line(it, &line_len);
      if (!line)
        return nullptr;
      if (line_len > 0 && line[line_len - 1] == '\n')
        line_len--;
      end = {this->selection.row, line_len};
      free(it->buffer);
      free(it);
      break;
    }
  }
  if (out_start)
    *out_start = start;
  uint32_t start_byte =
      line_to_byte(this->root, start.row, nullptr) + start.col;
  uint32_t end_byte = line_to_byte(this->root, end.row, nullptr) + end.col;
  char *text = read(this->root, start_byte, end_byte - start_byte);
  if (out_len)
    *out_len = end_byte - start_byte;
  return text;
}

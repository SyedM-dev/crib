#include "editor/editor.h"
#include "utils/utils.h"

Coord Editor::move_right(Coord cursor, uint32_t number) {
  Coord result = cursor;
  if (!this->root || number == 0)
    return result;
  uint32_t row = result.row;
  uint32_t col = result.col;
  uint32_t line_len = 0;
  LineIterator *it = begin_l_iter(this->root, row);
  if (!it)
    return result;
  char *line = next_line(it, &line_len);
  if (!line) {
    free(it->buffer);
    free(it);
    return result;
  }
  if (line_len > 0 && line[line_len - 1] == '\n')
    --line_len;
  while (number > 0) {
    if (col >= line_len) {
      uint32_t next_row = row + 1;
      if (next_row >= this->root->line_count) {
        col = line_len;
        break;
      }
      row = next_row;
      col = 0;
      line = next_line(it, &line_len);
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
  free(it->buffer);
  free(it);
  result.row = row;
  result.col = col;
  return result;
}

Coord Editor::move_left(Coord cursor, uint32_t number) {
  Coord result = cursor;
  if (!this->root || number == 0)
    return result;
  uint32_t row = result.row;
  uint32_t col = result.col;
  uint32_t len = 0;
  LineIterator *it = begin_l_iter(this->root, row);
  if (!it)
    return result;
  char *line = next_line(it, &len);
  if (!line) {
    free(it->buffer);
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
        prev_line(it, nullptr);
        iterator_ahead = false;
      }
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
  free(it->buffer);
  free(it);
  result.row = row;
  result.col = col;
  return result;
}

void Editor::cursor_down(uint32_t number) {
  if (!this->root || number == 0)
    return;
  uint32_t visual_col = this->cursor_preffered;
  if (visual_col == UINT32_MAX) {
    uint32_t len;
    LineIterator *it = begin_l_iter(this->root, this->cursor.row);
    char *line = next_line(it, &len);
    if (!line) {
      free(it->buffer);
      free(it);
      return;
    }
    this->cursor_preffered =
        get_visual_col_from_bytes(line, len, this->cursor.col);
    visual_col = this->cursor_preffered;
    free(it->buffer);
    free(it);
  }
  this->cursor.row = MIN(this->cursor.row + number, this->root->line_count - 1);
  uint32_t len;
  LineIterator *it = begin_l_iter(this->root, this->cursor.row);
  char *line = next_line(it, &len);
  if (!line) {
    free(it->buffer);
    free(it);
    return;
  }
  if (len > 0 && line[len - 1] == '\n')
    --len;
  this->cursor.col = get_bytes_from_visual_col(line, len, visual_col);
  free(it->buffer);
  free(it);
}

void Editor::cursor_up(uint32_t number) {
  if (!this->root || number == 0 || this->cursor.row == 0)
    return;
  uint32_t len;
  LineIterator *it = begin_l_iter(this->root, this->cursor.row);
  char *line_content = next_line(it, &len);
  if (!line_content)
    return;
  if (this->cursor_preffered == UINT32_MAX)
    this->cursor_preffered =
        get_visual_col_from_bytes(line_content, len, this->cursor.col);
  uint32_t visual_col = this->cursor_preffered;
  free(it->buffer);
  free(it);
  uint32_t target_row = this->cursor.row;
  while (number > 0 && target_row > 0) {
    target_row--;
    if (target_row == 0) {
      number--;
      break;
    }
    number--;
  }
  it = begin_l_iter(this->root, target_row);
  line_content = next_line(it, &len);
  if (line_content) {
    if (len > 0 && line_content[len - 1] == '\n')
      --len;
    this->cursor.row = target_row;
    this->cursor.col = get_bytes_from_visual_col(line_content, len, visual_col);
  } else {
    this->cursor.row = 0;
    this->cursor.col = 0;
  }
  free(it->buffer);
  free(it);
}

void Editor::cursor_right(uint32_t number) {
  if (!this->root || number == 0)
    return;
  this->cursor = this->move_right(this->cursor, number);
  this->cursor_preffered = UINT32_MAX;
}

void Editor::cursor_left(uint32_t number) {
  if (!this->root || number == 0)
    return;
  this->cursor = this->move_left(this->cursor, number);
  this->cursor_preffered = UINT32_MAX;
}

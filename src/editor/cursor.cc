#include "editor/editor.h"
#include "editor/folds.h"
#include "utils/utils.h"

Coord move_right_pure(Editor *editor, Coord cursor, uint32_t number) {
  Coord result = cursor;
  if (!editor || !editor->root || number == 0)
    return result;
  uint32_t row = result.row;
  uint32_t col = result.col;
  uint32_t line_len = 0;
  LineIterator *it = begin_l_iter(editor->root, row);
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
      if (next_row >= editor->root->line_count) {
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

Coord move_right(Editor *editor, Coord cursor, uint32_t number) {
  Coord result = cursor;
  if (!editor || !editor->root || number == 0)
    return result;
  uint32_t row = result.row;
  uint32_t col = result.col;
  uint32_t line_len = 0;
  LineIterator *it = begin_l_iter(editor->root, row);
  if (!it)
    return result;
  uint32_t target_row = next_unfolded_row(editor, row);
  while (row < target_row) {
    if (!next_line(it, &line_len)) {
      free(it->buffer);
      free(it);
      return result;
    }
    ++row;
  }
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
      uint32_t next_row = next_unfolded_row(editor, row + 1);
      if (next_row >= editor->root->line_count) {
        col = line_len;
        break;
      }
      while (row < next_row) {
        line = next_line(it, &line_len);
        if (!line) {
          free(it->buffer);
          free(it);
          result.row = row;
          result.col = col;
          return result;
        }
        ++row;
      }
      if (line_len > 0 && line[line_len - 1] == '\n')
        --line_len;
      col = 0;
      --number;
      continue;
    } else {
      uint32_t inc =
          grapheme_next_character_break_utf8(line + col, line_len - col);
      if (inc == 0)
        break;
      col += inc;
      --number;
    }
  }
  free(it->buffer);
  free(it);
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
      while (row > 0) {
        row--;
        line = prev_line(it, &len);
        if (!line)
          break;
        const Fold *fold = fold_for_line(editor->folds, row);
        if (fold) {
          while (line && row > fold->start) {
            line = prev_line(it, &len);
            row--;
          }
          line = nullptr;
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
  free(it->buffer);
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
  if (line_content == nullptr)
    return;
  if (editor->cursor_preffered == UINT32_MAX)
    editor->cursor_preffered =
        get_visual_col_from_bytes(line_content, len, editor->cursor.col);
  uint32_t visual_col = editor->cursor_preffered;
  free(it->buffer);
  free(it);
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
  if (!line_content)
    return;
  if (len > 0 && line_content[len - 1] == '\n')
    --len;
  editor->cursor.row = target_row;
  editor->cursor.col = get_bytes_from_visual_col(line_content, len, visual_col);
  free(it->buffer);
  free(it);
}

void cursor_up(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0 || editor->cursor.row == 0)
    return;
  uint32_t len;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
  char *line_content = next_line(it, &len);
  if (!line_content)
    return;
  if (editor->cursor_preffered == UINT32_MAX)
    editor->cursor_preffered =
        get_visual_col_from_bytes(line_content, len, editor->cursor.col);
  uint32_t visual_col = editor->cursor_preffered;
  free(it->buffer);
  free(it);
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
  if (line_content) {
    if (len > 0 && line_content[len - 1] == '\n')
      --len;
    editor->cursor.row = target_row;
    editor->cursor.col =
        get_bytes_from_visual_col(line_content, len, visual_col);
  } else {
    editor->cursor.row = 0;
    editor->cursor.col = 0;
  }
  free(it->buffer);
  free(it);
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

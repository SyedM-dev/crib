extern "C" {
#include "../libs/libgrapheme/grapheme.h"
}
#include "../include/editor.h"
#include "../include/main.h"
#include "../include/utils.h"

void handle_editor_event(Editor *editor, KeyEvent event) {
  if (event.key_type == KEY_SPECIAL) {
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
  }
  if (event.key_type == KEY_MOUSE) {
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
      }
      break;
    case PRESS:
      if (event.mouse_button == LEFT_BTN) {
        Coord p = editor_hit_test(editor, event.mouse_x, event.mouse_y);
        editor->cursor = p;
        editor->cursor_preffered = UINT32_MAX;
        editor->selection = p;
      }
      break;
    case DRAG:
      if (event.mouse_button == LEFT_BTN) {
        Coord p = editor_hit_test(editor, event.mouse_x, event.mouse_y);
        editor->cursor = p;
        editor->cursor_preffered = UINT32_MAX;
        editor->selection_active = true;
      }
      break;
    case RELEASE:
      if (event.mouse_button == LEFT_BTN)
        if (editor->cursor.row == editor->selection.row &&
            editor->cursor.col == editor->selection.col)
          editor->selection_active = false;
      break;
    }
  }
  switch (mode) {
  case NORMAL:
    if (event.key_type == KEY_CHAR && event.len == 1) {
      switch (event.c[0]) {
      case 'a':
      case 'i':
        mode = INSERT;
        break;
      case 's':
      case 'v':
        mode = SELECT;
        editor->selection_active = true;
        editor->selection = editor->cursor;
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
        } else if (isprint((unsigned char)(event.c[0]))) {
          edit_insert(editor, editor->cursor, event.c, 1);
          cursor_right(editor, 1);
        } else if (event.c[0] == 0x1B) {
          mode = NORMAL;
        }
      } else if (event.len > 1) {
        edit_insert(editor, editor->cursor, event.c, event.len);
        cursor_right(editor, 1);
      }
    }
    if (event.key_type == KEY_SPECIAL && event.special_key == KEY_DELETE)
      edit_erase(editor, editor->cursor, 1);
    break;
  case SELECT:
    if (event.key_type == KEY_CHAR && event.len == 1) {
      switch (event.c[0]) {
      case 0x1B:
      case 's':
      case 'v':
        editor->selection_active = false;
        mode = NORMAL;
        break;
      }
    }
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

Coord editor_hit_test(Editor *editor, uint32_t x, uint32_t y) {
  uint32_t target_visual_row = y;
  uint32_t visual_row = 0;
  uint32_t line_index = editor->scroll.row;
  bool first_visual_line = true;
  std::shared_lock knot_lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, line_index);
  if (!it)
    return editor->scroll;
  while (visual_row <= target_visual_row) {
    if (editor->folded[line_index]) {
      if (editor->folded[line_index] == 2) {
        if (visual_row == target_visual_row) {
          free(it);
          return {line_index, 0};
        }
        visual_row++;
      }
      do {
        char *l = next_line(it, nullptr);
        if (!l)
          break;
        free(l);
        line_index++;
      } while (editor->folded[line_index] == 1);
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
      while (left > 0 && col < editor->size.col) {
        uint32_t g =
            grapheme_next_character_break_utf8(line + offset + advance, left);
        int w = display_width(line + offset + advance, g);
        if (col + w > editor->size.col)
          break;
        if (visual_row == target_visual_row && x < col + w) {
          free(line);
          free(it);
          return {line_index, offset + advance + g};
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
      uint32_t next_row = row + 1;
      while (next_row < editor->root->line_count &&
             editor->folded[next_row] != 0)
        next_row++;
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
  free(it);
  if (!line)
    return result;
  if (len > 0 && line[len - 1] == '\n')
    --len;
  while (number > 0) {
    if (col == 0) {
      free(line);
      line = nullptr;
      if (row == 0)
        break;
      int32_t prev_row = row - 1;
      while (prev_row >= 0 && editor->folded[prev_row] != 0)
        prev_row--;
      if (prev_row < 0)
        break;
      row = prev_row;
      it = begin_l_iter(editor->root, row);
      line = next_line(it, &len);
      free(it);
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
  result.row = row;
  result.col = col;
  return result;
}

void cursor_down(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
  uint32_t len;
  char *line_content = next_line(it, &len);
  if (line_content == nullptr)
    return;
  if (editor->cursor_preffered == UINT32_MAX)
    editor->cursor_preffered =
        get_visual_col_from_bytes(line_content, len, editor->cursor.col);
  uint32_t visual_col = editor->cursor_preffered;
  do {
    free(line_content);
    line_content = next_line(it, &len);
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
  editor->cursor.col = get_bytes_from_visual_col(line_content, len, visual_col);
  free(line_content);
}

void cursor_up(Editor *editor, uint32_t number) {
  if (!editor || !editor->root || number == 0)
    return;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
  uint32_t len;
  char *line_content = next_line(it, &len);
  if (!line_content) {
    free(it);
    return;
  }
  if (editor->cursor_preffered == UINT32_MAX)
    editor->cursor_preffered =
        get_visual_col_from_bytes(line_content, len, editor->cursor.col);
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
  line_content = next_line(it, &len);
  if (!line_content) {
    free(it);
    return;
  }
  editor->cursor.col = get_bytes_from_visual_col(line_content, len, visual_col);
  free(line_content);
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
    Coord point = move_left(editor, pos, -len);
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
  } else {
    std::shared_lock lock_1(editor->knot_mtx);
    uint32_t cursor_original =
        line_to_byte(editor->root, editor->cursor.row, nullptr) +
        editor->cursor.col;
    TSPoint old_point = {pos.row, pos.col};
    uint32_t byte_pos = line_to_byte(editor->root, pos.row, nullptr) + pos.col;
    Coord point = move_right(editor, pos, len);
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
  if (memchr(data, '\n', len))
    editor->folded.resize(editor->root->line_count + 2);
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

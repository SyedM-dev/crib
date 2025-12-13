#include <cstdint>
extern "C" {
#include "../libs/libgrapheme/grapheme.h"
}
#include "../include/editor.h"
#include "../include/utils.h"

void scroll_up(Editor *editor, uint32_t number) {
  number++;
  Coord *scroll_queue = (Coord *)malloc(sizeof(Coord) * number);
  uint32_t q_head = 0;
  uint32_t q_size = 0;
  int line_index = editor->scroll.row - number;
  line_index = line_index < 0 ? 0 : line_index;
  LineIterator *it = begin_l_iter(editor->root, line_index);
  if (!it) {
    free(scroll_queue);
    return;
  }
  for (uint32_t i = line_index; i <= editor->scroll.row; i++) {
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line)
      break;
    if (line[line_len - 1] == '\n')
      --line_len;
    uint32_t offset = 0;
    uint32_t col = 0;
    if (q_size < number) {
      scroll_queue[(q_head + q_size) % number] = {i, 0};
      q_size++;
    } else {
      scroll_queue[q_head] = {i, 0};
      q_head = (q_head + 1) % number;
    }
    if (i == editor->scroll.row && 0 == editor->scroll.col) {
      editor->scroll = scroll_queue[q_head];
      free(line);
      free(it);
      free(scroll_queue);
      return;
    }
    while (offset < line_len) {
      uint32_t inc =
          grapheme_next_character_break_utf8(line + offset, line_len - offset);
      int width = display_width(line + offset, inc);
      if (col + width > editor->size.col) {
        if (q_size < number) {
          scroll_queue[(q_head + q_size) % number] = {i, offset};
          q_size++;
        } else {
          scroll_queue[q_head] = {i, offset};
          q_head = (q_head + 1) % number;
        }
        if (i == editor->scroll.row && offset >= editor->scroll.col) {
          editor->scroll = scroll_queue[q_head];
          free(line);
          free(it);
          free(scroll_queue);
          return;
        }
        col = 0;
      }
      col += width;
      offset += inc;
    }
    free(line);
  }
  editor->scroll = {0, 0};
  free(scroll_queue);
  free(it);
}

void scroll_down(Editor *editor, uint32_t number) {
  if (!editor || number == 0)
    return;
  uint32_t line_index = editor->scroll.row;
  LineIterator *it = begin_l_iter(editor->root, line_index);
  if (!it)
    return;
  const uint32_t max_visual_lines = editor->size.row;
  Coord *scroll_queue = (Coord *)malloc(sizeof(Coord) * max_visual_lines);
  uint32_t q_head = 0;
  uint32_t q_size = 0;
  uint32_t visual_seen = 0;
  bool first_visual_line = true;
  while (true) {
    if (editor->folded[line_index]) {
      if (editor->folded[line_index] == 2) {
        Coord fold_coord = {line_index, 0};
        if (q_size < max_visual_lines) {
          scroll_queue[(q_head + q_size) % max_visual_lines] = fold_coord;
          q_size++;
        } else {
          scroll_queue[q_head] = fold_coord;
          q_head = (q_head + 1) % max_visual_lines;
        }
        visual_seen++;
        if (visual_seen >= number + max_visual_lines) {
          editor->scroll = scroll_queue[q_head];
          break;
        }
      }
      do {
        char *line = next_line(it, nullptr);
        if (!line) {
          free(scroll_queue);
          free(it);
          return;
        }
        free(line);
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
    uint32_t current_byte_offset = 0;
    if (first_visual_line) {
      current_byte_offset += editor->scroll.col;
      first_visual_line = false;
    }
    while (current_byte_offset < line_len ||
           (line_len == 0 && current_byte_offset == 0)) {
      Coord coord = {line_index, current_byte_offset};
      if (q_size < max_visual_lines) {
        scroll_queue[(q_head + q_size) % max_visual_lines] = coord;
        q_size++;
      } else {
        scroll_queue[q_head] = coord;
        q_head = (q_head + 1) % max_visual_lines;
      }
      visual_seen++;
      if (visual_seen >= number + max_visual_lines) {
        editor->scroll = scroll_queue[q_head];
        free(line);
        free(scroll_queue);
        free(it);
        return;
      }
      uint32_t col = 0;
      uint32_t local_render_offset = 0;
      uint32_t left = line_len - current_byte_offset;
      while (left > 0 && col < editor->size.col) {
        uint32_t cluster_len = grapheme_next_character_break_utf8(
            line + current_byte_offset + local_render_offset, left);
        int width = display_width(
            line + current_byte_offset + local_render_offset, cluster_len);
        if (col + width > editor->size.col)
          break;
        local_render_offset += cluster_len;
        left -= cluster_len;
        col += width;
      }
      current_byte_offset += local_render_offset;
      if (line_len == 0)
        break;
    }
    free(line);
    line_index++;
  }
  if (q_size > 0) {
    uint32_t idx;
    if (q_size < max_visual_lines)
      idx = (q_head + q_size - 1) % max_visual_lines;
    else
      idx = q_head;
    editor->scroll = scroll_queue[idx];
  }
  free(it);
  free(scroll_queue);
}

void ensure_cursor(Editor *editor) {
  std::shared_lock knot_lock(editor->knot_mtx);
  if (editor->cursor.row < editor->scroll.row ||
      (editor->cursor.row == editor->scroll.row &&
       editor->cursor.col < editor->scroll.col)) {
    editor->cursor = editor->scroll;
    return;
  }
  uint32_t visual_rows = 0;
  uint32_t line_index = editor->scroll.row;
  bool first_visual_line = true;
  LineIterator *it = begin_l_iter(editor->root, line_index);
  if (!it)
    return;
  Coord last_visible = editor->scroll;
  while (true) {
    if (visual_rows >= editor->size.row)
      break;
    if (editor->folded[line_index]) {
      if (editor->folded[line_index] == 2) {
        Coord c = {line_index, 0};
        last_visible = c;
        visual_rows++;
        if (line_index == editor->cursor.row) {
          free(it);
          return;
        }
      }
      do {
        char *line = next_line(it, nullptr);
        if (!line)
          break;
        free(line);
        line_index++;
      } while (editor->folded[line_index] == 1);
      continue;
    }
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line)
      break;
    if (line_len > 0 && line[line_len - 1] == '\n')
      line_len--;
    uint32_t offset = first_visual_line ? editor->scroll.col : 0;
    first_visual_line = false;
    while (offset < line_len || (line_len == 0 && offset == 0)) {
      Coord current = {line_index, offset};
      last_visible = current;
      visual_rows++;
      if (visual_rows >= editor->size.row)
        break;
      uint32_t col = 0;
      uint32_t advance = 0;
      uint32_t left = line_len - offset;
      while (left > 0 && col < editor->size.col) {
        uint32_t g =
            grapheme_next_character_break_utf8(line + offset + advance, left);
        int w = display_width(line + offset + advance, g);
        if (col + w > editor->size.col)
          break;
        advance += g;
        left -= g;
        col += w;
      }
      if (line_index == editor->cursor.row) {
        if (editor->cursor.col >= offset &&
            editor->cursor.col < offset + advance) {
          free(line);
          free(it);
          return;
        }
      }
      if (advance == 0)
        break;
      offset += advance;
      if (line_len == 0)
        break;
    }
    free(line);
    line_index++;
  }
  editor->cursor = last_visible;
  free(it);
}

void ensure_scroll(Editor *editor) {
  std::shared_lock knot_lock(editor->knot_mtx);
  if (editor->cursor.row < editor->scroll.row ||
      (editor->cursor.row == editor->scroll.row &&
       editor->cursor.col < editor->scroll.col)) {
    LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
    if (!it)
      return;
    uint32_t len;
    char *line = next_line(it, &len);
    if (!line) {
      free(it);
      return;
    }
    if (len > 0 && line[len - 1] == '\n')
      --len;
    uint32_t rows = 1;
    uint32_t cols = 0;
    uint32_t offset = 0;
    uint32_t old_offset = 0;
    while (offset < len) {
      uint32_t inc =
          grapheme_next_character_break_utf8(line + offset, len - offset);
      int width = display_width(line + offset, inc);
      if (cols + width > editor->size.col) {
        rows++;
        cols = 0;
        if (editor->cursor.col > old_offset && editor->cursor.col <= offset) {
          editor->scroll.row = editor->cursor.row;
          editor->scroll.col = old_offset;
          free(line);
          free(it);
          return;
        }
        old_offset = offset;
      }
      cols += width;
      offset += inc;
    }
    free(line);
    free(it);
    editor->scroll.row = editor->cursor.row;
    editor->scroll.col = (editor->cursor.col == 0) ? 0 : old_offset;
  } else {
    uint32_t line_index = editor->scroll.row;
    LineIterator *it = begin_l_iter(editor->root, line_index);
    if (!it)
      return;
    uint32_t max_visual_lines = editor->size.row;
    Coord *scroll_queue = (Coord *)malloc(sizeof(Coord) * max_visual_lines);
    uint32_t q_head = 0;
    uint32_t q_size = 0;
    bool first_visual_line = true;
    while (true) {
      if (editor->folded[line_index]) {
        if (editor->folded[line_index] == 2) {
          Coord fold_coord = {line_index, 0};
          if (q_size < max_visual_lines) {
            scroll_queue[(q_head + q_size) % max_visual_lines] = fold_coord;
            q_size++;
          } else {
            scroll_queue[q_head] = fold_coord;
            q_head = (q_head + 1) % max_visual_lines;
          }
          if (line_index == editor->cursor.row) {
            editor->scroll = scroll_queue[q_head];
            break;
          }
        }
        do {
          char *line = next_line(it, nullptr);
          if (!line)
            break;
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
      if (first_visual_line) {
        current_byte_offset += editor->scroll.col;
        first_visual_line = false;
      }
      while (current_byte_offset < line_len ||
             (line_len == 0 && current_byte_offset == 0)) {
        Coord current_coord = {line_index, current_byte_offset};
        if (q_size < max_visual_lines) {
          scroll_queue[(q_head + q_size) % max_visual_lines] = current_coord;
          q_size++;
        } else {
          scroll_queue[q_head] = current_coord;
          q_head = (q_head + 1) % max_visual_lines;
        }
        uint32_t col = 0;
        uint32_t local_render_offset = 0;
        uint32_t line_left = line_len - current_byte_offset;
        while (line_left > 0 && col < editor->size.col) {
          uint32_t cluster_len = grapheme_next_character_break_utf8(
              line + current_byte_offset + local_render_offset, line_left);
          int width = display_width(
              line + current_byte_offset + local_render_offset, cluster_len);
          if (col + width > editor->size.col)
            break;
          local_render_offset += cluster_len;
          line_left -= cluster_len;
          col += width;
        }
        if (line_index == editor->cursor.row) {
          bool cursor_found = false;
          if (editor->cursor.col >= current_byte_offset &&
              editor->cursor.col < current_byte_offset + local_render_offset)
            cursor_found = true;
          else if (editor->cursor.col == line_len &&
                   current_byte_offset + local_render_offset == line_len)
            cursor_found = true;
          if (cursor_found) {
            editor->scroll = scroll_queue[q_head];
            free(line);
            free(scroll_queue);
            free(it);
            return;
          }
        }
        current_byte_offset += local_render_offset;
        if (line_len == 0)
          break;
      }
      line_index++;
      free(line);
    }
    free(scroll_queue);
    free(it);
  }
}

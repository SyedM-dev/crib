#include "editor/editor.h"

void Editor::ensure_cursor() {
  if (this->cursor < this->scroll) {
    this->cursor.row = this->scroll.row;
    this->cursor.col = this->scroll.col;
    this->cursor_preffered = UINT32_MAX;
    return;
  }
  uint32_t numlen =
      EXTRA_META + static_cast<int>(std::log10(this->root->line_count + 1));
  uint32_t render_width = this->size.col - numlen;
  uint32_t visual_rows = 0;
  uint32_t line_index = this->scroll.row;
  bool first_visual_line = true;
  LineIterator *it = begin_l_iter(this->root, line_index);
  if (!it)
    return;
  Coord last_visible = this->scroll;
  while (true) {
    if (visual_rows >= this->size.row)
      break;
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line)
      break;
    if (line_len > 0 && line[line_len - 1] == '\n')
      line_len--;
    uint32_t offset = first_visual_line ? this->scroll.col : 0;
    first_visual_line = false;
    while (offset < line_len || (line_len == 0 && offset == 0)) {
      Coord current = {line_index, offset};
      last_visible = current;
      visual_rows++;
      if (visual_rows >= this->size.row)
        break;
      uint32_t col = 0;
      uint32_t advance = 0;
      uint32_t left = line_len - offset;
      while (left > 0 && col < render_width) {
        uint32_t g =
            grapheme_next_character_break_utf8(line + offset + advance, left);
        int w = display_width(line + offset + advance, g);
        if (col + w > render_width)
          break;
        advance += g;
        left -= g;
        col += w;
      }
      if (line_index == this->cursor.row) {
        if (this->cursor.col >= offset &&
            this->cursor.col <= offset + advance) {
          free(it->buffer);
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
    line_index++;
  }
  this->cursor.row = last_visible.row;
  this->cursor.col = last_visible.col;
  this->cursor_preffered = UINT32_MAX;
  free(it->buffer);
  free(it);
}

void Editor::ensure_scroll() {
  uint32_t numlen =
      EXTRA_META + static_cast<int>(std::log10(this->root->line_count + 1));
  uint32_t render_width = this->size.col - numlen;
  if (this->cursor < this->scroll) {
    LineIterator *it = begin_l_iter(this->root, this->cursor.row);
    if (!it)
      return;
    uint32_t len;
    char *line = next_line(it, &len);
    if (!line) {
      free(it->buffer);
      free(it);
      return;
    }
    if (len > 0 && line[len - 1] == '\n')
      --len;
    uint32_t cols = 0;
    uint32_t offset = 0;
    uint32_t old_offset = 0;
    while (offset < len) {
      uint32_t inc =
          grapheme_next_character_break_utf8(line + offset, len - offset);
      int width = display_width(line + offset, inc);
      if (cols + width > render_width) {
        cols = 0;
        if (this->cursor.col > old_offset && this->cursor.col <= offset) {
          this->scroll.row = this->cursor.row;
          this->scroll.col = old_offset;
          free(it->buffer);
          free(it);
          return;
        }
        old_offset = offset;
      }
      cols += width;
      offset += inc;
    }
    free(it->buffer);
    free(it);
    this->scroll.row = this->cursor.row;
    this->scroll.col = (this->cursor.col == 0) ? 0 : old_offset;
  } else if (this->cursor.row - this->scroll.row < this->size.row * 2) {
    uint32_t line_index = this->scroll.row;
    LineIterator *it = begin_l_iter(this->root, line_index);
    if (!it)
      return;
    uint32_t max_visual_lines = this->size.row;
    Coord *scroll_queue = (Coord *)malloc(sizeof(Coord) * max_visual_lines);
    uint32_t q_head = 0;
    uint32_t q_size = 0;
    bool first_visual_line = true;
    while (true) {
      uint32_t line_len;
      char *line = next_line(it, &line_len);
      if (!line)
        break;
      if (line_len > 0 && line[line_len - 1] == '\n')
        line_len--;
      uint32_t current_byte_offset = 0;
      if (first_visual_line) {
        current_byte_offset += this->scroll.col;
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
        while (line_left > 0 && col < render_width) {
          uint32_t cluster_len = grapheme_next_character_break_utf8(
              line + current_byte_offset + local_render_offset, line_left);
          int width = display_width(
              line + current_byte_offset + local_render_offset, cluster_len);
          if (col + width > render_width)
            break;
          local_render_offset += cluster_len;
          line_left -= cluster_len;
          col += width;
        }
        if (line_index == this->cursor.row) {
          bool cursor_found = false;
          if (this->cursor.col >= current_byte_offset &&
              this->cursor.col < current_byte_offset + local_render_offset)
            cursor_found = true;
          else if (this->cursor.col == line_len &&
                   current_byte_offset + local_render_offset == line_len)
            cursor_found = true;
          if (cursor_found) {
            this->scroll = scroll_queue[q_head];
            free(scroll_queue);
            free(it->buffer);
            free(it);
            return;
          }
        }
        current_byte_offset += local_render_offset;
        if (line_len == 0)
          break;
      }
      line_index++;
    }
    free(scroll_queue);
    free(it->buffer);
    free(it);
  } else {
    this->scroll.row = (this->cursor.row > this->size.row * 1.5)
                           ? this->cursor.row - this->size.row * 1.5
                           : 0;
    this->scroll.col = 0;
    this->ensure_scroll();
  }
}

#include "editor/editor.h"

void scroll_up(Editor *editor, int32_t number) {
  if (!editor || number == 0)
    return;
  uint32_t numlen =
      EXTRA_META + static_cast<int>(std::log10(editor->root->line_count + 1));
  uint32_t render_width = editor->size.col - numlen;
  uint32_t line_index = editor->scroll.row;
  LineIterator *it = begin_l_iter(editor->root, line_index);
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
    len--;
  uint32_t current_byte_offset = 0;
  uint32_t col = 0;
  std::vector<uint32_t> segment_starts;
  segment_starts.reserve(16);
  if (current_byte_offset < editor->scroll.col)
    segment_starts.push_back(0);
  while (current_byte_offset < editor->scroll.col &&
         current_byte_offset < len) {
    uint32_t cluster_len = grapheme_next_character_break_utf8(
        line + current_byte_offset, len - current_byte_offset);
    int width = display_width(line + current_byte_offset, cluster_len);
    if (col + width > render_width) {
      segment_starts.push_back(current_byte_offset);
      col = 0;
    }
    current_byte_offset += cluster_len;
    col += width;
  }
  for (auto it_seg = segment_starts.rbegin(); it_seg != segment_starts.rend();
       ++it_seg) {
    if (--number == 0) {
      editor->scroll = {line_index, *it_seg};
      free(it->buffer);
      free(it);
      return;
    }
  }
  line = prev_line(it, &len);
  if (!line) {
    editor->scroll = {0, 0};
    free(it->buffer);
    free(it);
    return;
  }
  do {
    line_index--;
    line = prev_line(it, &len);
    if (!line) {
      editor->scroll = {0, 0};
      free(it->buffer);
      free(it);
      return;
    }
    if (len > 0 && line[len - 1] == '\n')
      len--;
    current_byte_offset = 0;
    col = 0;
    std::vector<uint32_t> segment_starts;
    segment_starts.reserve(16);
    segment_starts.push_back(0);
    while (current_byte_offset < len) {
      uint32_t cluster_len = grapheme_next_character_break_utf8(
          line + current_byte_offset, len - current_byte_offset);
      int width = display_width(line + current_byte_offset, cluster_len);
      if (col + width > render_width) {
        segment_starts.push_back(current_byte_offset);
        col = 0;
      }
      current_byte_offset += cluster_len;
      col += width;
    }
    for (auto it_seg = segment_starts.rbegin(); it_seg != segment_starts.rend();
         ++it_seg) {
      if (--number == 0) {
        editor->scroll = {line_index, *it_seg};
        free(it->buffer);
        free(it);
        return;
      }
    }
  } while (number > 0);
  free(it->buffer);
  free(it);
}

void scroll_down(Editor *editor, uint32_t number) {
  if (!editor || number == 0)
    return;
  uint32_t numlen =
      EXTRA_META + static_cast<int>(std::log10(editor->root->line_count + 1));
  uint32_t render_width = editor->size.col - numlen;
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
        free(scroll_queue);
        free(it->buffer);
        free(it);
        return;
      }
      uint32_t col = 0;
      uint32_t local_render_offset = 0;
      uint32_t left = line_len - current_byte_offset;
      while (left > 0 && col < render_width) {
        uint32_t cluster_len = grapheme_next_character_break_utf8(
            line + current_byte_offset + local_render_offset, left);
        int width = display_width(
            line + current_byte_offset + local_render_offset, cluster_len);
        if (col + width > render_width)
          break;
        local_render_offset += cluster_len;
        left -= cluster_len;
        col += width;
      }
      current_byte_offset += local_render_offset;
      if (line_len == 0)
        break;
    }
    line_index++;
  }
  if (q_size > 0) {
    uint32_t advance = (q_size > number) ? number : (q_size - 1);
    editor->scroll = scroll_queue[(q_head + advance) % max_visual_lines];
  }
  free(it->buffer);
  free(it);
  free(scroll_queue);
}

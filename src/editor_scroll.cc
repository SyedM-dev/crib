extern "C" {
#include "../libs/libgrapheme/grapheme.h"
}
#include "../include/editor.h"
#include "../include/utils.h"
#include <cmath>

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
    const Fold *fold = fold_for_line(editor->folds, line_index);
    if (fold) {
      while (line && line_index > fold->start) {
        free(line);
        line = prev_line(it, &len);
        line_index--;
        if (!line) {
          editor->scroll = {0, 0};
          free(it->buffer);
          free(it);
          return;
        }
      }
      if (--number == 0) {
        editor->scroll = {fold->start, 0};
        free(it->buffer);
        free(it);
        return;
      }
      if (fold->start == 0) {
        editor->scroll = {0, 0};
        free(it->buffer);
        free(it);
        return;
      }
      line_index = fold->start - 1;
      line = prev_line(it, &len);
      if (!line) {
        editor->scroll = {0, 0};
        free(it->buffer);
        free(it);
        return;
      }
      continue;
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
    const Fold *fold = fold_for_line(editor->folds, line_index);
    if (fold) {
      Coord fold_coord = {fold->start, 0};
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
      uint32_t skip_until = fold->end;
      while (line_index <= skip_until) {
        char *line = next_line(it, nullptr);
        if (!line) {
          free(scroll_queue);
          free(it->buffer);
          free(it);
          return;
        }
        line_index++;
      }
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

void ensure_cursor(Editor *editor) {
  std::shared_lock knot_lock(editor->knot_mtx);
  if (editor->cursor < editor->scroll) {
    uint32_t line_idx = next_unfolded_row(editor, editor->scroll.row);
    editor->cursor.row = line_idx;
    editor->cursor.col =
        line_idx == editor->scroll.row ? editor->scroll.col : 0;
    editor->cursor_preffered = UINT32_MAX;
    return;
  }
  uint32_t numlen =
      EXTRA_META + static_cast<int>(std::log10(editor->root->line_count + 1));
  uint32_t render_width = editor->size.col - numlen;
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
    const Fold *fold = fold_for_line(editor->folds, line_index);
    if (fold) {
      Coord c = {fold->start, 0};
      last_visible = c;
      visual_rows++;
      uint32_t skip_until = fold->end;
      while (line_index <= skip_until) {
        char *line = next_line(it, nullptr);
        if (!line)
          break;
        line_index++;
      }
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
      if (line_index == editor->cursor.row) {
        if (editor->cursor.col >= offset &&
            editor->cursor.col <= offset + advance) {
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
  uint32_t last_real_row = last_visible.row;
  const Fold *last_fold = fold_for_line(editor->folds, last_visible.row);
  if (last_fold) {
    last_visible.row = last_fold->start == 0 ? 0 : last_fold->start - 1;
    last_visible.col = 0;
  }
  editor->cursor.row = last_visible.row;
  editor->cursor.col = last_visible.row == last_real_row ? last_visible.col : 0;
  editor->cursor_preffered = UINT32_MAX;
  free(it->buffer);
  free(it);
}

void ensure_scroll(Editor *editor) {
  std::shared_lock knot_lock(editor->knot_mtx);
  uint32_t numlen =
      EXTRA_META + static_cast<int>(std::log10(editor->root->line_count + 1));
  uint32_t render_width = editor->size.col - numlen;
  if (editor->cursor < editor->scroll) {
    LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
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
    uint32_t rows = 1;
    uint32_t cols = 0;
    uint32_t offset = 0;
    uint32_t old_offset = 0;
    while (offset < len) {
      uint32_t inc =
          grapheme_next_character_break_utf8(line + offset, len - offset);
      int width = display_width(line + offset, inc);
      if (cols + width > render_width) {
        rows++;
        cols = 0;
        if (editor->cursor.col > old_offset && editor->cursor.col <= offset) {
          editor->scroll.row = editor->cursor.row;
          editor->scroll.col = old_offset;
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
      const Fold *fold = fold_for_line(editor->folds, line_index);
      if (fold) {
        Coord fold_coord = {fold->start, 0};
        if (q_size < max_visual_lines) {
          scroll_queue[(q_head + q_size) % max_visual_lines] = fold_coord;
          q_size++;
        } else {
          scroll_queue[q_head] = fold_coord;
          q_head = (q_head + 1) % max_visual_lines;
        }
        if (fold->start <= editor->cursor.row &&
            editor->cursor.row <= fold->end) {
          editor->scroll = scroll_queue[q_head];
          break;
        }
        uint32_t skip_until = fold->end;
        while (line_index <= skip_until) {
          char *line = next_line(it, nullptr);
          if (!line)
            break;
          line_index++;
        }
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
  }
}

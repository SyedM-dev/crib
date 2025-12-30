#include "editor/editor.h"
#include "editor/folds.h"
#include "main.h"

Coord editor_hit_test(Editor *editor, uint32_t x, uint32_t y) {
  if (mode == INSERT)
    x++;
  uint32_t numlen =
      EXTRA_META + static_cast<int>(std::log10(editor->root->line_count + 1));
  bool is_gutter_click = (x < numlen);
  uint32_t render_width = editor->size.col - numlen;
  x = MAX(x, numlen) - numlen;
  uint32_t target_visual_row = y;
  uint32_t visual_row = 0;
  uint32_t line_index = editor->scroll.row;
  uint32_t last_line_index = editor->scroll.row;
  uint32_t last_col = editor->scroll.col;
  bool first_visual_line = true;
  std::shared_lock knot_lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, line_index);
  if (!it)
    return editor->scroll;
  while (visual_row <= target_visual_row) {
    const Fold *fold = fold_for_line(editor->folds, line_index);
    if (fold) {
      if (visual_row == target_visual_row) {
        free(it->buffer);
        free(it);
        if (is_gutter_click) {
          remove_fold(editor, fold->start);
          return {UINT32_MAX, UINT32_MAX};
        }
        return {fold->start > 0 ? fold->start - 1 : 0, 0};
      }
      visual_row++;
      while (line_index <= fold->end) {
        char *l = next_line(it, nullptr);
        if (!l)
          break;
        line_index++;
      }
      last_line_index = fold->end;
      last_col = 0;
      continue;
    }
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line)
      break;
    if (line_len && line[line_len - 1] == '\n')
      line_len--;
    last_line_index = line_index;
    last_col = line_len;
    uint32_t offset = first_visual_line ? editor->scroll.col : 0;
    first_visual_line = false;
    while (offset < line_len || (line_len == 0 && offset == 0)) {
      uint32_t col = 0;
      uint32_t advance = 0;
      uint32_t left = line_len - offset;
      uint32_t last_good_offset = offset;
      while (left > 0 && col < render_width) {
        uint32_t g =
            grapheme_next_character_break_utf8(line + offset + advance, left);
        int w = display_width(line + offset + advance, g);
        if (col + w > render_width)
          break;
        if (visual_row == target_visual_row && x < col + w) {
          free(it->buffer);
          free(it);
          return {line_index, offset + advance};
        }
        advance += g;
        last_good_offset = offset + advance;
        left -= g;
        col += w;
      }
      last_col = last_good_offset;
      if (visual_row == target_visual_row) {
        free(it->buffer);
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
    line_index++;
  }
  free(it->buffer);
  free(it);
  return {last_line_index, last_col};
}

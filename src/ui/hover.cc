#include "ui/hover.h"
#include "syntax/decl.h"

void HoverBox::clear() {
  text = "";
  scroll_ = 0;
  is_markup = false;
  size = {0, 0};
  cells.clear();
}

void HoverBox::scroll(int32_t number) {
  if (text.empty() || number == 0)
    return;
  uint32_t line_count = 0;
  for (uint32_t i = 0; i < text.length(); i++)
    if (text[i] == '\n')
      line_count++;
  scroll_ = MAX((int32_t)scroll_ + number, 0);
  if (scroll_ > line_count)
    scroll_ = line_count;
  render_first(true);
}

void HoverBox::render_first(bool scroll) {
  if (!scroll) {
  }
  uint32_t longest_line = 0;
  uint32_t current_width = 0;
  for (size_t j = 0; j < text.length(); j++) {
    if (text[j] == '\n') {
      longest_line = std::max(longest_line, current_width);
      current_width = 0;
    } else {
      current_width += 1;
    }
  }
  // HACK: the 1 is added so the longest line doesnt wrap which should be fixed
  //       in the loop instead as it was never meant to wrap in the first place
  longest_line = MAX(longest_line, current_width) + 1;
  uint32_t content_width = MIN(longest_line, 130u);
  size.col = content_width + 2;
  size_t i = 0;
  size_t lines_skipped = 0;
  while (i < text.length() && lines_skipped < scroll_) {
    if (text[i] == '\n')
      lines_skipped++;
    i++;
  }
  uint32_t border_fg = 0x82AAFF;
  uint32_t base_bg = 0;
  cells.assign(size.col * 26, ScreenCell{" ", 0, 0, 0, 0, 0});
  auto set = [&](uint32_t r, uint32_t c, const char *text, uint32_t fg,
                 uint32_t bg, uint8_t flags) {
    cells[r * size.col + c] = {std::string(text), 0, fg, bg, flags, 0};
  };
  uint32_t r = 0;
  while (i < text.length() && r < 24) {
    uint32_t c = 0;
    while (c < content_width && i < text.length()) {
      if (text[i] == '\n') {
        while (i < text.length() && text[i] == '\n')
          i++;
        break;
      }
      uint32_t cluster_len = grapheme_next_character_break_utf8(
          text.c_str() + i, text.length() - i);
      std::string cluster = text.substr(i, cluster_len);
      int width = display_width(cluster.c_str(), cluster_len);
      if (c + width > content_width)
        break;
      // TODO: Use new highlights
      Highlight *hl = nullptr;
      uint32_t fg = hl ? hl->fg : 0xFFFFFF;
      uint32_t bg = hl ? hl->bg : 0;
      uint32_t flags = hl ? hl->flags : 0;
      set(r + 1, c + 1, cluster.c_str(), fg, bg | base_bg, flags);
      c += width;
      i += cluster_len;
      for (int w = 1; w < width; w++)
        set(r + 1, c - w + 1, "\x1b", 0xFFFFFF, base_bg, 0);
    }
    r++;
  }
  if (!scroll)
    size.row = r + 2;
  set(0, 0, "┌", border_fg, base_bg, 0);
  for (uint32_t i = 1; i < size.col - 1; i++)
    set(0, i, "─", border_fg, base_bg, 0);
  set(0, size.col - 1, "┐", border_fg, base_bg, 0);
  for (uint32_t r = 1; r < size.row - 1; r++) {
    set(r, 0, "│", border_fg, base_bg, 0);
    set(r, size.col - 1, "│", border_fg, base_bg, 0);
  }
  set(size.row - 1, 0, "└", border_fg, base_bg, 0);
  for (uint32_t i = 1; i < size.col - 1; i++)
    set(size.row - 1, i, "─", border_fg, base_bg, 0);
  set(size.row - 1, size.col - 1, "┘", border_fg, base_bg, 0);
  cells.resize(size.col * size.row);
}

void HoverBox::render(Coord pos) {
  int32_t start_row = (int32_t)pos.row - (int32_t)size.row;
  if (start_row < 0)
    start_row = pos.row + 1;
  int32_t start_col = pos.col;
  Coord screen_size = get_size();
  if (start_col + size.col > screen_size.col) {
    start_col = screen_size.col - size.col;
    if (start_col < 0)
      start_col = 0;
  }
  for (uint32_t r = 0; r < size.row; r++)
    for (uint32_t c = 0; c < size.col; c++)
      update(start_row + r, start_col + c, cells[r * size.col + c].utf8,
             cells[r * size.col + c].fg, cells[r * size.col + c].bg,
             cells[r * size.col + c].flags);
}

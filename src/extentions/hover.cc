#include "extentions/hover.h"
#include "syntax/decl.h"
#include "windows/decl.h"

TileRoot *init_hover() {
  auto root = std::make_unique<TileRoot>();
  root->tile = std::make_unique<HoverBox>();
  root->pos = {0, 0};
  root->size = {1, 1};
  root->tile->hidden = true;
  popups.push_back(std::move(root));
  return popups.back().get();
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
  scroll_dirty = true;
}

void HoverBox::render(std::vector<ScreenCell> &buffer, Coord size, Coord pos) {
  if (scroll_dirty) {
    // TODO: call syntax highlighter here
  }
  // int32_t start_row = (int32_t)pos.row - (int32_t)size.row;
  // if (start_row < 0)
  //   start_row = pos.row + 1;
  // int32_t start_col = pos.col; // pos here is the cursor pos
  Coord screen_size = get_size();
  // if (start_col + size.col > screen_size.col) {
  //   start_col = screen_size.col - size.col;
  //   if (start_col < 0)
  //     start_col = 0;
  // }
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
  // HACK: the 1 is added so the longest line doesnt wrap which should befixed
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
  auto set = [&](uint32_t r, uint32_t c, const char *text, uint32_t fg,
                 uint32_t bg, uint8_t flags, uint32_t width) {
    if (r < 0 || r >= size.row || c < 0 || c >= size.col)
      return;
    r += pos.row;
    c += pos.col;
    ScreenCell &cell = buffer[r * screen_size.col + c];
    cell.utf8 = text;
    cell.width = width;
    cell.fg = fg;
    cell.bg = bg;
    cell.flags = flags;
    cell.ul_color = 0;
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
      set(r + 1, c + 1, cluster.c_str(), fg, bg | base_bg, flags, width);
      c += width;
      i += cluster_len;
      for (int w = 1; w < width; w++)
        set(r + 1, c - w + 1, "\x1b", 0xFFFFFF, base_bg, 0, 0);
    }
    r++;
  }
  if (scroll_dirty)
    size.row = r + 2;
  set(0, 0, "┌", border_fg, base_bg, 0, 1);
  for (uint32_t i = 1; i < size.col - 1; i++)
    set(0, i, "─", border_fg, base_bg, 0, 1);
  set(0, size.col - 1, "┐", border_fg, base_bg, 0, 1);
  for (uint32_t r = 1; r < size.row - 1; r++) {
    set(r, 0, "│", border_fg, base_bg, 0, 1);
    set(r, size.col - 1, "│", border_fg, base_bg, 0, 1);
  }
  set(size.row - 1, 0, "└", border_fg, base_bg, 0, 1);
  for (uint32_t i = 1; i < size.col - 1; i++)
    set(size.row - 1, i, "─", border_fg, base_bg, 0, 1);
  set(size.row - 1, size.col - 1, "┘", border_fg, base_bg, 0, 1);
  scroll_dirty = false;
}

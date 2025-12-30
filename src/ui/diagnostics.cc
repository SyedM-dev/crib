#include "ui/diagnostics.h"

void DiagnosticBox::clear() {
  warnings.clear();
  cells.clear();
  box_width = 0;
  box_height = 0;
}

void DiagnosticBox::render_first() {
  if (warnings.empty())
    return;
  uint32_t longest_line = 8 + warnings[0].source.length();
  for (auto &warn : warnings) {
    longest_line = MAX(longest_line, (uint32_t)warn.text.length() + 7);
    longest_line = MAX(longest_line, (uint32_t)warn.code.length() + 4);
    for (auto &see_also : warn.see_also)
      longest_line = MAX(longest_line, (uint32_t)see_also.length() + 4);
  }
  uint32_t content_width = MIN(longest_line, 150u);
  box_width = content_width + 2;
  cells.assign(box_width * 25, {" ", 0, 0, 0, 0, 0});
  auto set = [&](uint32_t r, uint32_t c, const char *text, uint32_t fg,
                 uint32_t bg, uint8_t flags) {
    cells[r * box_width + c] = {std::string(text), 0, fg, bg, flags, 0};
  };
  uint32_t base_bg = 0;
  uint32_t border_fg = 0x82AAFF;
  uint32_t r = 0;
  if (warnings[0].source != "") {
    std::string src_txt = "Source: ";
    for (uint32_t i = 0; i < src_txt.length() && i < content_width; i++)
      set(1, i + 1, (char[2]){src_txt[i], 0}, 0x3EAAFF, base_bg, 0);
    for (uint32_t i = 0; i < warnings[0].source.length() && i < content_width;
         i++)
      set(1, i + 1 + src_txt.length(), (char[2]){warnings[0].source[i], 0},
          0xffffff, base_bg, 0);
    r++;
  }
  int idx = 1;
  for (auto &warn : warnings) {
    char buf[4];
    std::snprintf(buf, sizeof(buf), "%2d", idx % 100);
    std::string line_txt = std::string(buf) + ". ";
    for (uint32_t i = 0; i < line_txt.length(); i++)
      set(r + 1, i + 1, (char[2]){line_txt[i], 0}, 0xffffff, base_bg, 0);
    if (r >= 23)
      break;
    const char *err_sym = "";
    uint32_t c_sym = 0xAAAAAA;
    switch (warn.type) {
    case 1:
      err_sym = "";
      c_sym = 0xFF0000;
      break;
    case 2:
      err_sym = "";
      c_sym = 0xFFFF00;
      break;
    case 3:
      err_sym = "";
      c_sym = 0xFF00FF;
      break;
    case 4:
      err_sym = "";
      c_sym = 0xAAAAAA;
      break;
    }
    std::string text = warn.text_full + " " + err_sym;
    uint32_t i = 0;
    while (i < text.length() && r < 23) {
      uint32_t c = 4;
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
        set(r + 1, c + 1, cluster.c_str(), c_sym, base_bg, 0);
        c += width;
        i += cluster_len;
        for (int w = 1; w < width; w++)
          set(r + 1, c - w + 1, "\x1b", c_sym, base_bg, 0);
      }
      r++;
    }
    if (r >= 23)
      break;
    if (warn.code != "") {
      for (uint32_t i = 0; i < warn.code.length() && i + 5 < content_width; i++)
        set(r + 1, i + 5, (char[2]){warn.code[i], 0}, 0x81cdc6, base_bg, 0);
      r++;
    }
    if (r >= 23)
      break;
    for (std::string &see_also : warn.see_also) {
      uint32_t fg = 0xB55EFF;
      uint8_t colon_count = 0;
      for (uint32_t i = 0; i < see_also.length() && i + 5 < content_width;
           i++) {
        set(r + 1, i + 5, (char[2]){see_also[i], 0}, fg, base_bg, 0);
        if (see_also[i] == ':')
          colon_count++;
        if (colon_count == 2)
          fg = 0xFFFFFF;
      }
      r++;
      if (r >= 23)
        break;
    };
    idx++;
  }
  box_height = 2 + r;
  set(0, 0, "┌", border_fg, base_bg, 0);
  for (uint32_t i = 1; i < box_width - 1; i++)
    set(0, i, "─", border_fg, base_bg, 0);
  set(0, box_width - 1, "┐", border_fg, base_bg, 0);
  for (uint32_t r = 1; r < box_height - 1; r++) {
    set(r, 0, "│", border_fg, base_bg, 0);
    set(r, box_width - 1, "│", border_fg, base_bg, 0);
  }
  set(box_height - 1, 0, "└", border_fg, base_bg, 0);
  for (uint32_t i = 1; i < box_width - 1; i++)
    set(box_height - 1, i, "─", border_fg, base_bg, 0);
  set(box_height - 1, box_width - 1, "┘", border_fg, base_bg, 0);
  cells.resize(box_width * box_height);
}

void DiagnosticBox::render(Coord pos) {
  int32_t start_row = (int32_t)pos.row - (int32_t)box_height;
  if (start_row < 0)
    start_row = pos.row + 1;
  int32_t start_col = pos.col;
  if (start_col + box_width > cols) {
    start_col = cols - box_width;
    if (start_col < 0)
      start_col = 0;
  }
  for (uint32_t r = 0; r < box_height; r++)
    for (uint32_t c = 0; c < box_width; c++)
      update(start_row + r, start_col + c, cells[r * box_width + c].utf8,
             cells[r * box_width + c].fg, cells[r * box_width + c].bg,
             cells[r * box_width + c].flags);
}

// #include "ui/diagnostics.h"
//
// void DiagnosticBox::clear() {
//   warnings.clear();
//   cells.clear();
//   size = {0, 0};
// };
//
// void DiagnosticBox::render_first() {
//   if (warnings.empty())
//     return;
//   uint32_t longest_line = 8 + warnings[0].source.length();
//   for (auto &warn : warnings) {
//     uint32_t longest = 0;
//     uint32_t cur = 0;
//     for (char ch : warn.text_full)
//       if (ch == '\n') {
//         longest = MAX(longest, cur);
//         cur = 0;
//       } else {
//         if (ch == '\t')
//           cur += 3;
//         ++cur;
//       }
//     longest = MAX(longest, cur);
//     longest_line = MAX(longest_line, longest + 7);
//     longest_line = MAX(longest_line, (uint32_t)warn.code.length() + 4);
//     for (auto &see_also : warn.see_also)
//       longest_line = MAX(longest_line, (uint32_t)see_also.length() + 4);
//   }
//   uint32_t content_width = MIN(longest_line, 150u);
//   size.col = content_width + 2;
//   cells.assign(size.col * 25, {" ", 0, 0, 0, 0, 0});
//   auto set = [&](uint32_t r, uint32_t c, const char *text, uint32_t fg,
//                  uint32_t bg, uint8_t flags) {
//     cells[r * size.col + c] = {std::string(text), 0, fg, bg, flags, 0};
//   };
//   uint32_t base_bg = 0;
//   uint32_t border_fg = 0x82AAFF;
//   uint32_t r = 0;
//   if (warnings[0].source != "") {
//     std::string src_txt = "Source: ";
//     for (uint32_t i = 0; i < src_txt.length() && i < content_width; i++)
//       set(1, i + 1, (char[2]){src_txt[i], 0}, 0x3EAAFF, base_bg, 0);
//     for (uint32_t i = 0; i < warnings[0].source.length() && i <
//     content_width;
//          i++)
//       set(1, i + 1 + src_txt.length(), (char[2]){warnings[0].source[i], 0},
//           0xffffff, base_bg, 0);
//     r++;
//   }
//   int idx = 1;
//   for (auto &warn : warnings) {
//     char buf[4];
//     std::snprintf(buf, sizeof(buf), "%2d", idx % 100);
//     std::string line_txt = std::string(buf) + ". ";
//     for (uint32_t i = 0; i < line_txt.length(); i++)
//       set(r + 1, i + 1, (char[2]){line_txt[i], 0}, 0xffffff, base_bg, 0);
//     if (r >= 23)
//       break;
//     const char *err_sym = "";
//     uint32_t c_sym = 0xAAAAAA;
//     switch (warn.type) {
//     case 1:
//       err_sym = "";
//       c_sym = 0xFF0000;
//       break;
//     case 2:
//       err_sym = "";
//       c_sym = 0xFFFF00;
//       break;
//     case 3:
//       err_sym = "";
//       c_sym = 0xFF00FF;
//       break;
//     case 4:
//       err_sym = "";
//       c_sym = 0xAAAAAA;
//       break;
//     }
//     std::string text = warn.text_full + " " + err_sym;
//     uint32_t i = 0;
//     while (i < text.length() && r < 23) {
//       uint32_t c = 4;
//       while (c < content_width && i < text.length()) {
//         if (text[i] == '\n') {
//           while (i < text.length() && text[i] == '\n')
//             i++;
//           break;
//         }
//         uint32_t cluster_len = grapheme_next_character_break_utf8(
//             text.c_str() + i, text.length() - i);
//         std::string cluster = text.substr(i, cluster_len);
//         int width = display_width(cluster.c_str(), cluster_len);
//         if (c + width > content_width)
//           break;
//         set(r + 1, c + 1, cluster.c_str(), c_sym, base_bg, 0);
//         c += width;
//         i += cluster_len;
//         for (int w = 1; w < width; w++)
//           set(r + 1, c - w + 1, "\x1b", c_sym, base_bg, 0);
//       }
//       r++;
//     }
//     if (r >= 23)
//       break;
//     if (warn.code != "") {
//       for (uint32_t i = 0; i < warn.code.length() && i + 5 < content_width;
//       i++)
//         set(r + 1, i + 5, (char[2]){warn.code[i], 0}, 0x81cdc6, base_bg, 0);
//       r++;
//     }
//     if (r >= 23)
//       break;
//     for (std::string &see_also : warn.see_also) {
//       uint32_t fg = 0xB55EFF;
//       uint8_t colon_count = 0;
//       for (uint32_t i = 0; i < see_also.length() && i + 5 < content_width;
//            i++) {
//         set(r + 1, i + 5, (char[2]){see_also[i], 0}, fg, base_bg, 0);
//         if (see_also[i] == ':')
//           colon_count++;
//         if (colon_count == 2)
//           fg = 0xFFFFFF;
//       }
//       r++;
//       if (r >= 23)
//         break;
//     };
//     idx++;
//   }
//   size.row = 2 + r;
//   set(0, 0, "┌", border_fg, base_bg, 0);
//   for (uint32_t i = 1; i < size.col - 1; i++)
//     set(0, i, "─", border_fg, base_bg, 0);
//   set(0, size.col - 1, "┐", border_fg, base_bg, 0);
//   for (uint32_t r = 1; r < size.row - 1; r++) {
//     set(r, 0, "│", border_fg, base_bg, 0);
//     set(r, size.col - 1, "│", border_fg, base_bg, 0);
//   }
//   set(size.row - 1, 0, "└", border_fg, base_bg, 0);
//   for (uint32_t i = 1; i < size.col - 1; i++)
//     set(size.row - 1, i, "─", border_fg, base_bg, 0);
//   set(size.row - 1, size.col - 1, "┘", border_fg, base_bg, 0);
//   cells.resize(size.col * size.row);
// }
//
// void DiagnosticBox::render(Coord pos) {
//   int32_t start_row = (int32_t)pos.row - (int32_t)size.row;
//   if (start_row < 0)
//     start_row = pos.row + 1;
//   int32_t start_col = pos.col;
//   Coord screen_size = get_size();
//   if (start_col + size.col > screen_size.col) {
//     start_col = screen_size.col - size.col;
//     if (start_col < 0)
//       start_col = 0;
//   }
//   for (uint32_t r = 0; r < size.row; r++)
//     for (uint32_t c = 0; c < size.col; c++)
//       update(start_row + r, start_col + c, cells[r * size.col + c].utf8,
//              cells[r * size.col + c].fg, cells[r * size.col + c].bg,
//              cells[r * size.col + c].flags);
// }

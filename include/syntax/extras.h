#ifndef SYNTAX_EXTRAS_H
#define SYNTAX_EXTRAS_H

#include "io/knot.h"
#include "syntax/decl.h"
#include "utils/utils.h"

inline static const std::vector<std::pair<std::string, uint32_t>> color_map = {
    {"AliceBlue", 0xF0F8FF},
    {"AntiqueWhite", 0xFAEBD7},
    {"Aqua", 0x00FFFF},
    {"Aquamarine", 0x7FFFD4},
    {"Azure", 0xF0FFFF},
    {"Beige", 0xF5F5DC},
    {"Bisque", 0xFFE4C4},
    {"Black", 0x000000},
    {"BlanchedAlmond", 0xFFEBCD},
    {"Blue", 0x0000FF},
    {"BlueViolet", 0x8A2BE2},
    {"Brown", 0xA52A2A},
    {"BurlyWood", 0xDEB887},
    {"CadetBlue", 0x5F9EA0},
    {"Chartreuse", 0x7FFF00},
    {"Chocolate", 0xD2691E},
    {"Coral", 0xFF7F50},
    {"CornflowerBlue", 0x6495ED},
    {"Cornsilk", 0xFFF8DC},
    {"Crimson", 0xDC143C},
    {"Cyan", 0x00FFFF},
    {"DarkBlue", 0x00008B},
    {"DarkCyan", 0x008B8B},
    {"DarkGoldenRod", 0xB8860B},
    {"DarkGray", 0xA9A9A9},
    {"DarkGrey", 0xA9A9A9},
    {"DarkGreen", 0x006400},
    {"DarkKhaki", 0xBDB76B},
    {"DarkMagenta", 0x8B008B},
    {"DarkOliveGreen", 0x556B2F},
    {"DarkOrange", 0xFF8C00},
    {"DarkOrchid", 0x9932CC},
    {"DarkRed", 0x8B0000},
    {"DarkSalmon", 0xE9967A},
    {"DarkSeaGreen", 0x8FBC8F},
    {"DarkSlateBlue", 0x483D8B},
    {"DarkSlateGray", 0x2F4F4F},
    {"DarkSlateGrey", 0x2F4F4F},
    {"DarkTurquoise", 0x00CED1},
    {"DarkViolet", 0x9400D3},
    {"DeepPink", 0xFF1493},
    {"DeepSkyBlue", 0x00BFFF},
    {"DimGray", 0x696969},
    {"DimGrey", 0x696969},
    {"DodgerBlue", 0x1E90FF},
    {"FireBrick", 0xB22222},
    {"FloralWhite", 0xFFFAF0},
    {"ForestGreen", 0x228B22},
    {"Fuchsia", 0xFF00FF},
    {"Gainsboro", 0xDCDCDC},
    {"GhostWhite", 0xF8F8FF},
    {"Gold", 0xFFD700},
    {"GoldenRod", 0xDAA520},
    {"Gray", 0x808080},
    {"Grey", 0x808080},
    {"Green", 0x008000},
    {"GreenYellow", 0xADFF2F},
    {"HoneyDew", 0xF0FFF0},
    {"HotPink", 0xFF69B4},
    {"IndianRed", 0xCD5C5C},
    {"Indigo", 0x4B0082},
    {"Ivory", 0xFFFFF0},
    {"Khaki", 0xF0E68C},
    {"Lavender", 0xE6E6FA},
    {"LavenderBlush", 0xFFF0F5},
    {"LawnGreen", 0x7CFC00},
    {"LemonChiffon", 0xFFFACD},
    {"LightBlue", 0xADD8E6},
    {"LightCoral", 0xF08080},
    {"LightCyan", 0xE0FFFF},
    {"LightGoldenRodYellow", 0xFAFAD2},
    {"LightGray", 0xD3D3D3},
    {"LightGrey", 0xD3D3D3},
    {"LightGreen", 0x90EE90},
    {"LightPink", 0xFFB6C1},
    {"LightSalmon", 0xFFA07A},
    {"LightSeaGreen", 0x20B2AA},
    {"LightSkyBlue", 0x87CEFA},
    {"LightSlateGray", 0x778899},
    {"LightSlateGrey", 0x778899},
    {"LightSteelBlue", 0xB0C4DE},
    {"LightYellow", 0xFFFFE0},
    {"Lime", 0x00FF00},
    {"LimeGreen", 0x32CD32},
    {"Linen", 0xFAF0E6},
    {"Magenta", 0xFF00FF},
    {"Maroon", 0x800000},
    {"MediumAquaMarine", 0x66CDAA},
    {"MediumBlue", 0x0000CD},
    {"MediumOrchid", 0xBA55D3},
    {"MediumPurple", 0x9370DB},
    {"MediumSeaGreen", 0x3CB371},
    {"MediumSlateBlue", 0x7B68EE},
    {"MediumSpringGreen", 0x00FA9A},
    {"MediumTurquoise", 0x48D1CC},
    {"MediumVioletRed", 0xC71585},
    {"MidnightBlue", 0x191970},
    {"MintCream", 0xF5FFFA},
    {"MistyRose", 0xFFE4E1},
    {"Moccasin", 0xFFE4B5},
    {"NavajoWhite", 0xFFDEAD},
    {"Navy", 0x000080},
    {"OldLace", 0xFDF5E6},
    {"Olive", 0x808000},
    {"OliveDrab", 0x6B8E23},
    {"Orange", 0xFFA500},
    {"OrangeRed", 0xFF4500},
    {"Orchid", 0xDA70D6},
    {"PaleGoldenRod", 0xEEE8AA},
    {"PaleGreen", 0x98FB98},
    {"PaleTurquoise", 0xAFEEEE},
    {"PaleVioletRed", 0xDB7093},
    {"PapayaWhip", 0xFFEFD5},
    {"PeachPuff", 0xFFDAB9},
    {"Peru", 0xCD853F},
    {"Pink", 0xFFC0CB},
    {"Plum", 0xDDA0DD},
    {"PowderBlue", 0xB0E0E6},
    {"Purple", 0x800080},
    {"RebeccaPurple", 0x663399},
    {"Red", 0xFF0000},
    {"RosyBrown", 0xBC8F8F},
    {"RoyalBlue", 0x4169E1},
    {"SaddleBrown", 0x8B4513},
    {"Salmon", 0xFA8072},
    {"SandyBrown", 0xF4A460},
    {"SeaGreen", 0x2E8B57},
    {"SeaShell", 0xFFF5EE},
    {"Sienna", 0xA0522D},
    {"Silver", 0xC0C0C0},
    {"SkyBlue", 0x87CEEB},
    {"SlateBlue", 0x6A5ACD},
    {"SlateGray", 0x708090},
    {"SlateGrey", 0x708090},
    {"Snow", 0xFFFAFA},
    {"SpringGreen", 0x00FF7F},
    {"SteelBlue", 0x4682B4},
    {"Tan", 0xD2B48C},
    {"Teal", 0x008080},
    {"Thistle", 0xD8BFD8},
    {"Tomato", 0xFF6347},
    {"Turquoise", 0x40E0D0},
    {"Violet", 0xEE82EE},
    {"Wheat", 0xF5DEB3},
    {"White", 0xFFFFFF},
    {"WhiteSmoke", 0xF5F5F5},
    {"Yellow", 0xFFFF00},
    {"YellowGreen", 0x9ACD32},
};

// Add word under cursor to this

struct ExtraHighlighter {
  std::vector<uint32_t> colors;
  std::array<std::vector<uint32_t>, 50> lines;
  Trie<uint32_t> css_colors = Trie<uint32_t>();
  uint32_t start = 0;

  ExtraHighlighter() { css_colors.build(color_map, false); }

  void render(Knot *root, uint32_t n_start, std::string word, bool is_css) {
    start = n_start;
    for (auto &line : lines)
      line.clear();
    LineIterator *it = begin_l_iter(root, start);
    if (!it)
      return;
    uint32_t idx = 0;
    uint32_t len;
    char *line;
    while (idx < 50 && (line = next_line(it, &len))) {
      lines[idx].assign(len, UINT32_MAX - 1);
      uint32_t i = 0;
      while (i < len) {
        if (is_css) {
          std::optional<uint32_t> color;
          uint32_t color_len = css_colors.match(
              line, i, len, [](char c) { return isalnum(c) || c == '_'; },
              &color);
          if (color) {
            for (uint32_t j = 0; j < color_len; j++)
              lines[idx][i + j] = *color;
            i += color_len;
            continue;
          } else if (i + 5 < len && (line[i] == 'r' || line[i] == 'R') &&
                     (line[i + 1] == 'g' || line[i + 1] == 'G') &&
                     (line[i + 2] == 'b' || line[i + 2] == 'B')) {
            uint32_t start = i;
            i += 3;
            if (line[i] == 'a' || line[i] == 'A')
              i++;
            if (line[i] == '(') {
              i++;
              bool is_percent = false;
              std::string r = "";
              while (i < len && line[i] >= '0' && line[i] <= '9')
                r += line[i++];
              if (r.empty())
                continue;
              while (i < len &&
                     (line[i] == '.' || (line[i] >= '0' && line[i] <= '9')))
                i++;
              if (line[i] == '%') {
                is_percent = true;
                i++;
              }
              while (i < len && (line[i] == ',' || line[i] == ' '))
                i++;
              std::string g = "";
              while (i < len && line[i] >= '0' && line[i] <= '9')
                g += line[i++];
              if (g.empty())
                continue;
              while (i < len &&
                     (line[i] == '.' || (line[i] >= '0' && line[i] <= '9')))
                i++;
              while (i < len &&
                     (line[i] == ',' || line[i] == ' ' || line[i] == '%'))
                i++;
              std::string b = "";
              while (i < len && line[i] >= '0' && line[i] <= '9')
                b += line[i++];
              if (b.empty())
                continue;
              while (i < len &&
                     (line[i] == ',' || line[i] == ' ' || line[i] == '.' ||
                      line[i] == '/' || line[i] == '%' ||
                      (line[i] >= '0' && line[i] <= '9')))
                i++;
              if (i < len && line[i] == ')')
                i++;
              else
                continue;
              uint32_t rr, gg, bb;
              if (is_percent) {
                rr = std::stoul(r) * 255 / 100;
                gg = std::stoul(g) * 255 / 100;
                bb = std::stoul(b) * 255 / 100;
              } else {
                rr = std::stoul(r);
                gg = std::stoul(g);
                bb = std::stoul(b);
              }
              rr = rr > 255 ? 255 : rr;
              gg = gg > 255 ? 255 : gg;
              bb = bb > 255 ? 255 : bb;
              uint32_t color = (rr << 16) | (gg << 8) | bb;
              for (uint32_t j = start; j < i; j++)
                lines[idx][j] = color;
            }
            continue;
          } else if (i + 5 < len && (line[i] == 'h' || line[i] == 'H') &&
                     (line[i + 1] == 's' || line[i + 1] == 'S') &&
                     (line[i + 2] == 'l' || line[i + 2] == 'L')) {
            uint32_t start = i;
            i += 3;
            if (line[i] == 'a' || line[i] == 'A')
              i++;
            if (line[i] == '(') {
              i++;
              std::string h = "";
              std::string h_unit = "";
              enum unit : uint8_t { deg, grad, rad, turn };
              unit u = deg;
              bool negative = false;
              if (i < len && (line[i] == '-' || line[i] == '+')) {
                negative = line[i] == '-';
                i++;
              }
              while (i < len && line[i] >= '0' && line[i] <= '9')
                h += line[i++];
              if (i < len && line[i] == '.') {
                h += '.';
                while (i < len && line[i] >= '0' && line[i] <= '9')
                  h += line[i++];
              }
              if (h.empty())
                continue;
              while (i < len && ((line[i] >= 'a' && line[i] <= 'z') ||
                                 (line[i] >= 'A' && line[i] <= 'Z')))
                h_unit += line[i++];
              for (size_t x = 0; x < h_unit.size(); x++)
                h_unit[x] = tolower(h_unit[x]);
              if (h_unit.empty())
                u = deg;
              else if (h_unit == "deg")
                u = deg;
              else if (h_unit == "grad")
                u = grad;
              else if (h_unit == "rad")
                u = rad;
              else if (h_unit == "turn")
                u = turn;
              else
                continue;
              double hue = std::stod(h);
              if (negative)
                hue = -hue;
              switch (u) {
              case deg:
                break;
              case grad:
                hue = hue * 360.0 / 400.0;
                break;
              case rad:
                hue = hue * 180.0 / M_PI;
                break;
              case turn:
                hue = hue * 360.0;
                break;
              }
              hue = fmod(hue, 360.0);
              if (hue < 0)
                hue += 360.0;
              double h_final = hue / 360.0;
              while (i < len && (line[i] == ',' || line[i] == ' '))
                i++;
              std::string s = "";
              while (i < len && line[i] >= '0' && line[i] <= '9')
                s += line[i++];
              if (s.empty())
                continue;
              if (i < len && line[i] == '%')
                i++;
              else
                continue;
              while (i < len && (line[i] == ',' || line[i] == ' '))
                i++;
              std::string l = "";
              while (i < len && line[i] >= '0' && line[i] <= '9')
                l += line[i++];
              if (l.empty())
                continue;
              if (i < len && line[i] == '%')
                i++;
              else
                continue;
              while (i < len &&
                     (line[i] == ',' || line[i] == ' ' || line[i] == '.' ||
                      line[i] == '/' || line[i] == '%' ||
                      (line[i] >= '0' && line[i] <= '9')))
                i++;
              if (i < len && line[i] == ')')
                i++;
              double s_val = std::stod(s) / 100.0;
              double l_val = std::stod(l) / 100.0;
              uint32_t color = hslToRgb(h_final, s_val, l_val);
              for (uint32_t j = start; j < i; j++)
                lines[idx][j] = color;
            }
            continue;
          }
        }
        if (i + 4 < len && line[i] == '#') {
          i++;
          uint32_t start = i;
          while (i < len && isxdigit(line[i]))
            i++;
          uint32_t color = 0;
          if (is_css && (i - start == 3 || i - start == 4)) {
            uint32_t r =
                std::stoul(std::string(line + start, 1), nullptr, 16) * 0x11;
            uint32_t g =
                std::stoul(std::string(line + start + 1, 1), nullptr, 16) *
                0x11;
            uint32_t b =
                std::stoul(std::string(line + start + 2, 1), nullptr, 16) *
                0x11;
            color = (r << 16) | (g << 8) | b;
          } else if ((is_css && (i - start == 8)) || i - start == 6) {
            uint32_t r = std::stoul(std::string(line + start, 2), nullptr, 16);
            uint32_t g =
                std::stoul(std::string(line + start + 2, 2), nullptr, 16);
            uint32_t b =
                std::stoul(std::string(line + start + 4, 2), nullptr, 16);
            color = (r << 16) | (g << 8) | b;
          } else {
            continue;
          }
          for (uint32_t j = start - 1; j < i; j++)
            lines[idx][j] = color;
          continue;
        } else if (i + 5 < len && line[i] == '0' && line[i + 1] == 'x') {
          i += 2;
          uint32_t start = i;
          while (i < len && isxdigit(line[i]))
            i++;
          uint32_t color = 0;
          if (i - start == 6) {
            uint32_t r = std::stoul(std::string(line + start, 2), nullptr, 16);
            uint32_t g =
                std::stoul(std::string(line + start + 2, 2), nullptr, 16);
            uint32_t b =
                std::stoul(std::string(line + start + 4, 2), nullptr, 16);
            color = (r << 16) | (g << 8) | b;
          } else {
            continue;
          }
          if (color)
            color--;
          else
            color++;
          for (uint32_t j = start - 2; j < i; j++)
            lines[idx][j] = color;
          continue;
        }
        if (i < len && (isalnum(line[i]) || line[i] == '_')) {
          uint32_t start = i;
          uint32_t x = 0;
          bool found = true;
          while (i < len && (isalnum(line[i]) || line[i] == '_')) {
            if (x < word.size() && line[i] == word[x]) {
              i++;
              x++;
            } else {
              found = false;
              i++;
            }
          }
          if (found && x == word.size())
            for (uint32_t j = start; j < i; j++)
              lines[idx][j] = UINT32_MAX;
        } else {
          i += utf8_codepoint_width(line[i]);
        }
      }
      idx++;
    }
    free(it->buffer);
    free(it);
  }

  std::optional<std::pair<uint32_t, uint32_t>> get(Coord pos) {
    uint32_t val;
    if (pos.row < start || pos.row >= start + 50 ||
        pos.col >= lines[pos.row - start].size() ||
        (val = lines[pos.row - start][pos.col]) == UINT32_MAX - 1)
      return std::nullopt;
    return (std::pair<uint32_t, uint32_t>){fg_for_bg(val), val};
  }

private:
  uint32_t fg_for_bg(uint32_t color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    double luminance = 0.299 * r + 0.587 * g + 0.114 * b;
    return (luminance > 128) ? 0x000000 : 0xFFFFFF;
  }

  uint32_t hslToRgb(double h, double s, double l) {
    double r, g, b;
    if (s == 0.0) {
      r = g = b = l;
    } else {
      auto hue2rgb = [](double p, double q, double t) -> double {
        if (t < 0)
          t += 1;
        if (t > 1)
          t -= 1;
        if (t < 1.0 / 6)
          return p + (q - p) * 6 * t;
        if (t < 1.0 / 2)
          return q;
        if (t < 2.0 / 3)
          return p + (q - p) * (2.0 / 3 - t) * 6;
        return p;
      };
      double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
      double p = 2 * l - q;
      r = hue2rgb(p, q, h + 1.0 / 3);
      g = hue2rgb(p, q, h);
      b = hue2rgb(p, q, h - 1.0 / 3);
    }
    uint32_t R = static_cast<uint32_t>(std::clamp(r, 0.0, 1.0) * 255);
    uint32_t G = static_cast<uint32_t>(std::clamp(g, 0.0, 1.0) * 255);
    uint32_t B = static_cast<uint32_t>(std::clamp(b, 0.0, 1.0) * 255);
    return (R << 16) | (G << 8) | B;
  }
};

#endif

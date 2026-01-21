#include "io/sysio.h"

static uint32_t rows, cols;
static bool show_cursor = 0;
static std::vector<ScreenCell> screen;
static std::vector<ScreenCell> old_screen;
static std::mutex screen_mutex;
static termios orig_termios;

void disable_raw_mode() {
  std::string os = "\x1b[?1049l\x1b[2 q\x1b[?1002l\x1b[?25h\x1b[?2004l";
  write(STDOUT_FILENO, os.c_str(), os.size());
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    perror("tcsetattr");
    exit(EXIT_FAILURE);
  }
}

void enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    exit(EXIT_FAILURE);
  atexit(disable_raw_mode);
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    exit(EXIT_FAILURE);
  std::string os = "\x1b[?1049h\x1b[2 q\x1b[?1002h\x1b[?25l\x1b[?2004h";
  write(STDOUT_FILENO, os.c_str(), os.size());
}

Coord start_screen() {
  enable_raw_mode();
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  rows = w.ws_row;
  cols = w.ws_col;
  screen.assign(rows * cols, {});
  old_screen.assign(rows * cols, {});
  return {rows, cols};
}

void end_screen() { disable_raw_mode(); }

Coord get_size() { return {rows, cols}; }

void update(uint32_t row, uint32_t col, std::string utf8, uint32_t fg,
            uint32_t bg, uint8_t flags) {
  if (row >= rows || col >= cols)
    return;
  uint32_t idx = row * cols + col;
  std::lock_guard<std::mutex> lock(screen_mutex);
  screen[idx].utf8 = utf8 != "" ? utf8 : "";
  if (utf8 == "")
    return;
  screen[idx].width = display_width(utf8.c_str(), utf8.size());
  screen[idx].fg = fg;
  screen[idx].bg = bg;
  screen[idx].flags = flags;
  screen[idx].ul_color = 0;
}

void update(uint32_t row, uint32_t col, const char *utf8, uint32_t fg,
            uint32_t bg, uint8_t flags) {
  if (row >= rows || col >= cols)
    return;
  uint32_t idx = row * cols + col;
  std::lock_guard<std::mutex> lock(screen_mutex);
  screen[idx].utf8 = utf8 ? utf8 : "";
  if (utf8 == nullptr)
    return;
  screen[idx].width = display_width(utf8, strlen(utf8));
  screen[idx].fg = fg;
  screen[idx].bg = bg;
  screen[idx].flags = flags;
  screen[idx].ul_color = 0;
}

void update(uint32_t row, uint32_t col, std::string utf8, uint32_t fg,
            uint32_t bg, uint8_t flags, uint32_t ul_color) {
  if (row >= rows || col >= cols)
    return;
  uint32_t idx = row * cols + col;
  std::lock_guard<std::mutex> lock(screen_mutex);
  screen[idx].utf8 = utf8 != "" ? utf8 : "";
  if (utf8 == "")
    return;
  screen[idx].width = display_width(utf8.c_str(), utf8.size());
  screen[idx].fg = fg;
  screen[idx].bg = bg;
  screen[idx].flags = flags;
  screen[idx].ul_color = ul_color;
}

void update(uint32_t row, uint32_t col, const char *utf8, uint32_t fg,
            uint32_t bg, uint8_t flags, uint32_t ul_color) {
  if (row >= rows || col >= cols)
    return;
  uint32_t idx = row * cols + col;
  std::lock_guard<std::mutex> lock(screen_mutex);
  screen[idx].utf8 = utf8 ? utf8 : "";
  if (utf8 == nullptr)
    return;
  screen[idx].width = display_width(utf8, strlen(utf8));
  screen[idx].fg = fg;
  screen[idx].bg = bg;
  screen[idx].flags = flags;
  screen[idx].ul_color = ul_color;
}

void render() {
  static bool first_render = true;
  uint32_t current_fg = 0;
  uint32_t current_bg = 0;
  uint32_t current_ul_color = 0;
  bool current_italic = false;
  bool current_bold = false;
  bool current_strikethrough = false;
  bool current_underline = false;
  std::lock_guard<std::mutex> lock(screen_mutex);
  std::string out;
  out.reserve(static_cast<size_t>(rows) * static_cast<size_t>(cols) * 4 + 256);
  out += "\x1b[s\x1b[?25l";
  if (first_render) {
    out += "\x1b[2J\x1b[H";
    first_render = false;
  }
  for (uint32_t row = 0; row < rows; ++row) {
    int64_t first_change_col = -1;
    int64_t last_change_col = -1;
    for (uint32_t col = 0; col < cols; ++col) {
      uint32_t idx = row * cols + col;
      ScreenCell &old_cell = old_screen[idx];
      ScreenCell &new_cell = screen[idx];
      bool content_changed =
          old_cell.utf8 != new_cell.utf8 || old_cell.fg != new_cell.fg ||
          old_cell.bg != new_cell.bg || old_cell.flags != new_cell.flags ||
          old_cell.ul_color != new_cell.ul_color;
      if (content_changed) {
        if (first_change_col == -1) {
          first_change_col = col;
          if (first_change_col > 0)
            for (int64_t back = 1; back <= 4 && first_change_col - back >= 0;
                 ++back) {
              ScreenCell &prev_cell =
                  screen[row * cols + (first_change_col - back)];
              if (prev_cell.width > 1) {
                first_change_col -= back;
                break;
              }
            }
        }
        last_change_col = MIN(cols - 1, col + 4);
      }
    }
    if (first_change_col == -1)
      continue;
    char buf[64];
    snprintf(buf, sizeof(buf), "\x1b[%d;%ldH", row + 1, first_change_col + 1);
    out.append(buf);
    for (uint32_t col = first_change_col; col <= last_change_col; ++col) {
      uint32_t idx = row * cols + col;
      ScreenCell &old_cell = old_screen[idx];
      ScreenCell &new_cell = screen[idx];
      uint32_t width = new_cell.width > 0 ? new_cell.width : 1;
      bool overlap = false;
      if (width > 1) {
        for (uint32_t i = 1; i < width; ++i) {
          uint32_t next_col = col + i;
          if (next_col >= cols)
            break;
          const ScreenCell &next = screen[row * cols + next_col];
          if (!is_empty_cell(next)) {
            overlap = true;
            break;
          }
        }
      }
      if (current_fg != new_cell.fg) {
        if (new_cell.fg) {
          char fb[64];
          snprintf(fb, sizeof(fb), "\x1b[38;2;%d;%d;%dm",
                   (new_cell.fg >> 16) & 0xFF, (new_cell.fg >> 8) & 0xFF,
                   (new_cell.fg >> 0) & 0xFF);
          out.append(fb);
        } else {
          out += "\x1b[39m";
        }
        current_fg = new_cell.fg;
      }
      if (current_bg != new_cell.bg) {
        if (new_cell.bg) {
          char bb[64];
          snprintf(bb, sizeof(bb), "\x1b[48;2;%d;%d;%dm",
                   (new_cell.bg >> 16) & 0xFF, (new_cell.bg >> 8) & 0xFF,
                   (new_cell.bg >> 0) & 0xFF);
          out.append(bb);
        } else {
          out += "\x1b[49m";
        }
        current_bg = new_cell.bg;
      }
      bool italic = (new_cell.flags & CF_ITALIC) != 0;
      if (italic != current_italic) {
        out += italic ? "\x1b[3m" : "\x1b[23m";
        current_italic = italic;
      }
      bool bold = (new_cell.flags & CF_BOLD) != 0;
      if (bold != current_bold) {
        out += bold ? "\x1b[1m" : "\x1b[22m";
        current_bold = bold;
      }
      bool strikethrough = (new_cell.flags & CF_STRIKETHROUGH) != 0;
      if (strikethrough != current_strikethrough) {
        out += strikethrough ? "\x1b[9m" : "\x1b[29m";
        current_strikethrough = strikethrough;
      }
      bool underline = (new_cell.flags & CF_UNDERLINE) != 0;
      if (underline) {
        if (new_cell.ul_color != current_ul_color) {
          if (new_cell.ul_color) {
            char ubuf[64];
            snprintf(ubuf, sizeof(ubuf), "\x1b[58;2;%d;%d;%dm",
                     (new_cell.ul_color >> 16) & 0xFF,
                     (new_cell.ul_color >> 8) & 0xFF,
                     (new_cell.ul_color >> 0) & 0xFF);
            out.append(ubuf);
          } else {
            out += "\x1b[59m";
          }
          current_ul_color = new_cell.ul_color;
        }
      }
      if (underline != current_underline) {
        out += underline ? "\x1b[4m" : "\x1b[24m";
        current_underline = underline;
      }
      if (width > 1 && overlap) {
        for (uint32_t i = 1; i < width; ++i) {
          uint32_t next_col = col + i;
          if (next_col >= cols)
            break;
          const ScreenCell &next = screen[row * cols + next_col];
          if (!is_empty_cell(next))
            break;
          out.push_back(' ');
        }
        out.push_back(' ');
      } else {
        if (!new_cell.utf8.empty()) {
          if (new_cell.utf8[0] == '\t') {
            out.append("    ");
          } else if (new_cell.utf8[0] != '\x1b') {
            out.append(new_cell.utf8);
          } else if (new_cell.utf8[0] == '\x1b') {
            ScreenCell *cell = &new_cell;
            int back = 0;
            while ((int)col - back >= 0 && cell->utf8[0] == '\x1b')
              cell = &screen[row * cols + col - ++back];
            if (width >= cell->width)
              out.append(" ");
          }
        } else {
          out.push_back(' ');
        }
      }
      old_cell.utf8 = new_cell.utf8;
      old_cell.fg = new_cell.fg;
      old_cell.bg = new_cell.bg;
      old_cell.flags = new_cell.flags;
      old_cell.width = new_cell.width;
    }
  }
  out += "\x1b[0m";
  out += "\x1b[u";
  if (show_cursor)
    out += "\x1b[?25h";
  const char *ptr = out.data();
  size_t remaining = out.size();
  while (remaining > 0) {
    ssize_t written = write(STDOUT_FILENO, ptr, remaining);
    if (written == 0) {
      break;
    } else if (written == -1) {
      if (errno == EINTR)
        continue;
      exit(EXIT_FAILURE);
      break;
    } else {
      ptr += written;
      remaining -= written;
    }
  }
}

void set_cursor(uint8_t row, uint8_t col, uint32_t type,
                bool show_cursor_param) {
  char buf[32];
  uint32_t n = snprintf(buf, sizeof(buf), "\x1b[%d;%dH\x1b[%d q", row + 1,
                        col + 1, type);
  show_cursor = show_cursor_param;
  write(STDOUT_FILENO, buf, n);
}

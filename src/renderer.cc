// includes
extern "C" {
#include "../libs/libgrapheme/grapheme.h"
#include "../libs/unicode_width/unicode_width.h"
}
#include "../include/ui.h"

uint32_t rows, cols;
int show_cursor = 0;
std::vector<ScreenCell> screen;
std::vector<ScreenCell> old_screen;
std::mutex screen_mutex;
termios orig_termios;

int display_width(const char *str) {
  if (!str || !*str)
    return 0;
  if (str[0] == '\t')
    return 4;
  unicode_width_state_t state;
  unicode_width_init(&state);
  int width = 0;
  for (size_t j = 0; j < strlen(str); j++) {
    unsigned char c = str[j];
    if (c < 128) {
      int char_width = unicode_width_process(&state, c);
      if (char_width > 0)
        width += char_width;
    } else {
      uint_least32_t cp;
      size_t bytes = grapheme_decode_utf8(str + j, strlen(str) - j, &cp);
      if (bytes > 1) {
        int char_width = unicode_width_process(&state, cp);
        if (char_width > 0)
          width += char_width;
        j += bytes - 1;
      }
    }
  }
  return width;
}

void get_terminal_size() {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  rows = w.ws_row;
  cols = w.ws_col;
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
  raw.c_lflag |= ISIG;
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    exit(EXIT_FAILURE);

  std::string os = "\x1b[?1049h\x1b[2 q\x1b[?1002h\x1b[?25l";
  write(STDOUT_FILENO, os.c_str(), os.size());
}

void disable_raw_mode() {
  std::string os = "\x1b[?1049l\x1b[2 q\x1b[?1002l\x1b[?25h";
  write(STDOUT_FILENO, os.c_str(), os.size());
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    perror("tcsetattr");
    exit(EXIT_FAILURE);
  }
}

Coord start_screen() {
  enable_raw_mode();
  get_terminal_size();
  screen.assign(rows * cols, {});     // allocate & zero-init
  old_screen.assign(rows * cols, {}); // allocate & zero-init
  return {rows, cols};
}

void end_screen() { disable_raw_mode(); }

Coord get_size() { return {rows, cols}; }

void update(uint32_t row, uint32_t col, const char *utf8, uint32_t fg,
            uint32_t bg, uint8_t flags) {
  if (row >= rows || col >= cols)
    return;

  uint32_t idx = row * cols + col;
  std::lock_guard<std::mutex> lock(screen_mutex);

  screen[idx].utf8 = utf8 ? utf8 : "";
  screen[idx].fg = fg;
  screen[idx].bg = bg;
  screen[idx].flags = flags;
}

bool ends_with(const std::string &string_to_check) {
  size_t len = string_to_check.size();
  if (len < 3)
    return false;
  return (string_to_check[len - 3] == VS16_BYTE_A) &&
         (string_to_check[len - 2] == VS16_BYTE_B) &&
         (string_to_check[len - 1] == VS16_BYTE_C);
}

void render() {
  static bool first_render = true;
  uint32_t current_fg = 0;
  uint32_t current_bg = 0;
  bool current_italic = false;
  bool current_bold = false;
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
    int first_change_col = -1;
    int last_change_col = -1;
    for (uint32_t col = 0; col < cols; ++col) {
      uint32_t idx = row * cols + col;
      ScreenCell &old_cell = old_screen[idx];
      ScreenCell &new_cell = screen[idx];
      bool content_changed = old_cell.utf8 != new_cell.utf8;
      bool style_changed =
          (old_cell.fg != new_cell.fg) || (old_cell.bg != new_cell.bg) ||
          ((old_cell.flags & CF_ITALIC) != (new_cell.flags & CF_ITALIC)) ||
          ((old_cell.flags & CF_BOLD) != (new_cell.flags & CF_BOLD)) ||
          ((old_cell.flags & CF_UNDERLINE) != (new_cell.flags & CF_UNDERLINE));
      if (content_changed || style_changed) {
        if (first_change_col == -1)
          first_change_col = col;
        last_change_col = col;
      }
    }
    if (first_change_col == -1)
      continue;
    char buf[64];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", row + 1, first_change_col + 1);
    out.append(buf);
    for (int col = first_change_col; col <= last_change_col; ++col) {
      int idx = row * cols + col;
      ScreenCell &old_cell = old_screen[idx];
      ScreenCell &new_cell = screen[idx];
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
      bool underline = (new_cell.flags & CF_UNDERLINE) != 0;
      if (underline != current_underline) {
        out += underline ? "\x1b[4m" : "\x1b[24m";
        current_underline = underline;
      }
      if (!new_cell.utf8.empty()) {
        if (new_cell.utf8[0] == '\t') {
          out.append("    ");
        } else {
          // HACK: This is a hack to work around the fact that emojis should be
          //      double width but handling them as so requires a lot of
          //      calculations for word wrapping so eventually have to do that
          //      and render them as the 2 wide they should be.
          if (new_cell.utf8.size() > 1) {
            if (new_cell.utf8.size() >= 3 && ends_with(new_cell.utf8)) {
              out.append(new_cell.utf8.substr(0, new_cell.utf8.size() - 3));
              out.append("\xEF\xB8\x8E");
            } else {
              out.append(new_cell.utf8);
              out.append("\xEF\xB8\x8E");
            }
          } else {
            out.append(new_cell.utf8);
          }
        }
      } else {
        out.append(1, ' ');
      }
      old_cell.utf8 = new_cell.utf8;
      old_cell.fg = new_cell.fg;
      old_cell.bg = new_cell.bg;
      old_cell.flags = new_cell.flags;
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

void set_cursor(int row, int col, int show_cursor_param) {
  char buf[32];
  int n = snprintf(buf, sizeof(buf), "\x1b[%d;%dH\x1b[5 q", row + 1, col + 1);
  show_cursor = show_cursor_param;
  write(STDOUT_FILENO, buf, n);
}

// includes
#include "../../libs/libgrapheme/grapheme.h"
#include "../../libs/unicode_width/unicode_width.h"
#include "../headers/header.hpp"

struct termios orig_termios;

int rows, cols;
bool show_cursor = false;
std::vector<ScreenCell> screen;
std::vector<ScreenCell> old_screen;
std::mutex screen_mutex;

int real_width(std::string str) {
  if (!str.size())
    return 0;
  const char *p = str.c_str();
  if (str[0] == '\t')
    return 4;
  unicode_width_state_t state;
  unicode_width_init(&state);
  int width = 0;
  for (size_t j = 0; j < str.size(); j++) {
    unsigned char c = str[j];
    if (c < 128) {
      int char_width = unicode_width_process(&state, c);
      if (char_width > 0)
        width += char_width;
    } else {
      uint_least32_t cp;
      size_t bytes = grapheme_decode_utf8(p + j, str.size() - j, &cp);
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

void die(const char *s) {
  perror(s);
  disable_raw_mode();
  exit(EXIT_FAILURE);
}

void enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");
  atexit(disable_raw_mode);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");

  std::string os = "\x1b[?1049h\x1b[2 q\x1b[?1002h\x1b[?25l";
  write(STDOUT_FILENO, os.c_str(), os.size());
}

void disable_raw_mode() {
  std::string os = "\x1b[?1049l\x1b[?25h";
  write(STDOUT_FILENO, os.c_str(), os.size());
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    perror("tcsetattr");
    exit(EXIT_FAILURE);
  }
}

void start_screen() {
  enable_raw_mode();
  get_terminal_size();
  screen.assign(rows * cols, {});     // allocate & zero-init
  old_screen.assign(rows * cols, {}); // allocate & zero-init
}

void end_screen() { disable_raw_mode(); }

void update(int row, int col, const char *utf8, uint32_t fg, uint32_t bg,
            uint8_t flags) {
  if (row < 0 || row >= rows || col < 0 || col >= cols)
    return;

  int idx = row * cols + col;
  std::lock_guard<std::mutex> lock(screen_mutex);

  screen[idx].utf8 = utf8 ? utf8 : ""; // nullptr => empty string
  screen[idx].fg = fg;
  screen[idx].bg = bg;
  screen[idx].flags = flags;
}

coords get_size() { return {rows, cols}; }

void render() {
  static bool first_render = true;
  uint32_t current_fg = 0;
  uint32_t current_bg = 0;
  bool current_italic = false;
  bool current_bold = false;
  bool current_underline = false;

  std::lock_guard<std::mutex> lock(screen_mutex);

  std::string out;
  // reserve a conservative amount to avoid repeated reallocs
  out.reserve(static_cast<size_t>(rows) * static_cast<size_t>(cols) * 4 + 256);

  // save cursor + hide
  out += "\x1b[s\x1b[?25l";

  if (first_render) {
    out += "\x1b[2J\x1b[H";
    first_render = false;
  }

  for (int row = 0; row < rows; ++row) {
    int first_change_col = -1;
    int last_change_col = -1;

    // detect change span in this row
    for (int col = 0; col < cols; ++col) {
      int idx = row * cols + col;
      ScreenCell &old_cell = old_screen[idx];
      ScreenCell &new_cell = screen[idx];

      bool old_empty = old_cell.utf8.empty();
      bool new_empty = new_cell.utf8.empty();

      bool content_changed =
          (old_empty && !new_empty) || (!old_empty && new_empty) ||
          (!old_empty && !new_empty && old_cell.utf8 != new_cell.utf8);

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

    // move cursor once to the start of change region
    char buf[32];
    int n = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", row + 1,
                     first_change_col + 1);
    out.append(buf, n);

    // render changed region
    for (int col = first_change_col; col <= last_change_col; ++col) {
      int idx = row * cols + col;
      ScreenCell &old_cell = old_screen[idx];
      ScreenCell &new_cell = screen[idx];

      // foreground change
      if (current_fg != new_cell.fg) {
        if (new_cell.fg) {
          char fb[64];
          int m = snprintf(
              fb, sizeof(fb), "\x1b[38;2;%d;%d;%dm", (new_cell.fg >> 16) & 0xFF,
              (new_cell.fg >> 8) & 0xFF, (new_cell.fg >> 0) & 0xFF);
          out.append(fb, m);
        } else {
          out += "\x1b[39m";
        }
        current_fg = new_cell.fg;
      }

      // background change
      if (current_bg != new_cell.bg) {
        if (new_cell.bg) {
          char bb[64];
          int m = snprintf(
              bb, sizeof(bb), "\x1b[48;2;%d;%d;%dm", (new_cell.bg >> 16) & 0xFF,
              (new_cell.bg >> 8) & 0xFF, (new_cell.bg >> 0) & 0xFF);
          out.append(bb, m);
        } else {
          out += "\x1b[49m";
        }
        current_bg = new_cell.bg;
      }

      // italic
      bool italic = (new_cell.flags & CF_ITALIC) != 0;
      if (italic != current_italic) {
        out += italic ? "\x1b[3m" : "\x1b[23m";
        current_italic = italic;
      }

      // bold
      bool bold = (new_cell.flags & CF_BOLD) != 0;
      if (bold != current_bold) {
        out += bold ? "\x1b[1m" : "\x1b[22m";
        current_bold = bold;
      }

      // underline
      bool underline = (new_cell.flags & CF_UNDERLINE) != 0;
      if (underline != current_underline) {
        out += underline ? "\x1b[4m" : "\x1b[24m";
        current_underline = underline;
      }

      // content
      if (!new_cell.utf8.empty()) {
        if (new_cell.utf8[0] == '\t')
          out.append("    ");
        else
          out.append(new_cell.utf8);
      } else {
        out.append(1, ' ');
      }

      // copy new -> old (std::string assignment, no strdup/free)
      old_cell.utf8 = new_cell.utf8;
      old_cell.fg = new_cell.fg;
      old_cell.bg = new_cell.bg;
      old_cell.flags = new_cell.flags;
    }
  }

  // final reset + restore cursor + show cursor
  out += "\x1b[0m";
  out += "\x1b[u";
  if (show_cursor)
    out += "\x1b[?25h";

  // single syscall to write the whole frame
  ssize_t written = write(STDOUT_FILENO, out.data(), out.size());
  (void)written; // you may check for errors in debug builds
}

void set_cursor(int row, int col, bool show_cursor_param) {
  char buf[32];
  int n = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", row + 1, col + 1);
  show_cursor = show_cursor_param;
  write(STDOUT_FILENO, buf, n);
}

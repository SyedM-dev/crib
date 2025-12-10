#ifndef UI_H
#define UI_H

#include "./utils.h"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

#define KEY_CHAR 0
#define KEY_SPECIAL 1
#define KEY_MOUSE 2
#define KEY_NONE 3

#define KEY_UP 0
#define KEY_DOWN 1
#define KEY_LEFT 2
#define KEY_RIGHT 3
#define KEY_DELETE 4

#define KEY_ESC '\x1b'

#define PRESS 0
#define RELEASE 1
#define DRAG 2
#define SCROLL 3

#define LEFT_BTN 0
#define MIDDLE_BTN 1
#define RIGHT_BTN 2
#define SCROLL_BTN 3
#define NONE_BTN 4

#define SCROLL_UP 0
#define SCROLL_DOWN 1
#define SCROLL_LEFT 2
#define SCROLL_RIGHT 3

#define ALT 1
#define CNTRL 2
#define CNTRL_ALT 3
#define SHIFT 4

const char VS16_BYTE_A = '\xEF';
const char VS16_BYTE_B = '\xB8';
const char VS16_BYTE_C = '\x8F';

enum CellFlags : uint8_t {
  CF_NONE = 0,
  CF_ITALIC = 1 << 0,
  CF_BOLD = 1 << 1,
  CF_UNDERLINE = 1 << 2,
};

struct ScreenCell {
  std::string utf8 = std::string(""); // empty => no content
  uint32_t fg = 0;
  uint32_t bg = 0;
  uint8_t flags = CF_NONE;
};

struct KeyEvent {
  uint8_t key_type;

  char c;

  uint8_t special_key;
  uint8_t special_modifier;

  uint8_t mouse_x;
  uint8_t mouse_y;
  uint8_t mouse_button;
  uint8_t mouse_state;
  uint8_t mouse_direction;
  uint8_t mouse_modifier;
};

extern uint32_t rows, cols;
extern std::vector<ScreenCell> screen; // size rows*cols
extern std::vector<ScreenCell> old_screen;
extern std::mutex screen_mutex;
extern std::atomic<bool> running;

void get_terminal_size();
void enable_raw_mode();
void disable_raw_mode();
Coord start_screen();
void end_screen();
void update(uint32_t row, uint32_t col, const char *utf8, uint32_t fg,
            uint32_t bg, uint8_t flags);
void set_cursor(int row, int col, int show_cursor_param);
void render();
Coord get_size();

int read_input(char *buf, size_t buflen);
KeyEvent read_key();

int display_width(const char *str);

#endif

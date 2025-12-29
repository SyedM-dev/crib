#ifndef UI_H
#define UI_H

#include "./pch.h"
#include "./utils.h"
#include <cstdint>

#define KEY_CHAR 0
#define KEY_SPECIAL 1
#define KEY_MOUSE 2
#define KEY_PASTE 3
#define KEY_NONE 4

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

#define DEFAULT 0
#define BLOCK 2
#define UNDERLINE 4
#define CURSOR 6

enum CellFlags : uint8_t {
  CF_NONE = 0,
  CF_ITALIC = 1 << 0,
  CF_BOLD = 1 << 1,
  CF_UNDERLINE = 1 << 2
};

struct ScreenCell {
  std::string utf8 = std::string("");
  uint8_t width = 1;
  uint32_t fg = 0;
  uint32_t bg = 0;
  uint8_t flags = CF_NONE;
  uint32_t ul_color = 0;
};

struct KeyEvent {
  uint8_t key_type;

  char *c;
  uint32_t len;

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
extern std::vector<ScreenCell> screen;
extern std::vector<ScreenCell> old_screen;
extern std::mutex screen_mutex;

Coord start_screen();
void end_screen();
void update(uint32_t row, uint32_t col, std::string utf8, uint32_t fg,
            uint32_t bg, uint8_t flags);
void update(uint32_t row, uint32_t col, const char *utf8, uint32_t fg,
            uint32_t bg, uint8_t flags);
void update(uint32_t row, uint32_t col, std::string utf8, uint32_t fg,
            uint32_t bg, uint8_t flags, uint32_t ul_color);
void update(uint32_t row, uint32_t col, const char *utf8, uint32_t fg,
            uint32_t bg, uint8_t flags, uint32_t ul_color);
void set_cursor(int row, int col, int type, bool show_cursor_param);
void render();
Coord get_size();

KeyEvent read_key();

#endif

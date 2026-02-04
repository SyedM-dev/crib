#ifndef UI_H
#define UI_H

#include "pch.h"
#include "utils/utils.h"

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
  CF_UNDERLINE = 1 << 2,
  CF_STRIKETHROUGH = 1 << 3
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
  /* KEY_CHAR, KEY_SPECIAL, KEY_MOUSE, KEY_PASTE, KEY_NONE */
  uint8_t key_type;

  /* the character / string if key_type == KEY_CHAR or KEY_PASTE */
  char *c;
  /* length of c */
  uint32_t len;

  /* KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_DELETE if key_type ==
   * KEY_SPECIAL */
  uint8_t special_key;
  /* ALT, CNTRL, CNTRL_ALT, SHIFT if key_type == KEY_SPECIAL */
  uint8_t special_modifier;

  /* column of mouse click */
  uint8_t mouse_x;
  /* row of mouse click */
  uint8_t mouse_y;
  /* LEFT_BTN, MIDDLE_BTN, RIGHT_BTN, SCROLL_BTN, NONE_BTN if key_type ==
   * KEY_MOUSE */
  uint8_t mouse_button;
  /* PRESS, RELEASE, DRAG, SCROLL if key_type == KEY_MOUSE */
  uint8_t mouse_state;
  /* SCROLL_UP, SCROLL_DOWN, SCROLL_LEFT, SCROLL_RIGHT if key_type ==
   * KEY_MOUSE and mouse_state == SCROLL */
  uint8_t mouse_direction;
  /* ALT, CNTRL, CNTRL_ALT, SHIFT if key_type == KEY_MOUSE */
  uint8_t mouse_modifier;
};

inline bool is_empty_cell(const ScreenCell &c) {
  return c.utf8.empty() || c.utf8 == " " || c.utf8 == "\x1b";
}

extern std::vector<ScreenCell> new_screen;

Coord start_screen();
void end_screen();
void set_cursor(uint8_t row, uint8_t col, uint32_t type,
                bool show_cursor_param);
void io_render();
Coord get_size();

KeyEvent read_key();

#endif

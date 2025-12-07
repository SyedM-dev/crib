#include "../include/ui.h"

int read_input(char *buf, size_t buflen) {
  size_t i = 0;
  int n;
  n = read(STDIN_FILENO, &buf[i], 1);
  if (n <= 0)
    return -1;
  i++;
  if (buf[0] == '\x1b') {
    while (i < buflen - 1) {
      n = read(STDIN_FILENO, &buf[i], 1);
      if (n <= 0)
        break;
      i++;
    }
  }
  buf[i] = '\0';
  return i;
}

void capture_mouse(char *buf, KeyEvent *ret) {
  uint8_t byte = buf[3];
  ret->mouse_modifier = (byte >> 3) & 0x03;
  uint8_t aa = (byte >> 5) & 0x03;
  uint8_t cc = byte & 0x03;
  ret->mouse_x = buf[4] - 33;
  ret->mouse_y = buf[5] - 33;
  ret->mouse_direction = 4;
  if (aa == 1 && cc == 3) {
    ret->mouse_state = RELEASE;
    ret->mouse_button = NONE_BTN;
  } else if (aa == 1) {
    ret->mouse_state = PRESS;
    ret->mouse_button = cc;
  } else if (aa == 2) {
    ret->mouse_state = DRAG;
    ret->mouse_button = cc;
  } else if (aa == 3) {
    ret->mouse_button = SCROLL_BTN;
    ret->mouse_state = SCROLL;
    ret->mouse_direction = cc;
  } else {
    ret->mouse_state = RELEASE;
    ret->mouse_button = NONE_BTN;
  }
}

KeyEvent read_key_nonblock() {
  KeyEvent ret;
  char buf[7];
  int n = read_input(buf, sizeof(buf));
  if (n <= 0) {
    ret.key_type = KEY_NONE;
    ret.c = '\0';
    return ret;
  }
  if (n == 1) {
    ret.key_type = KEY_CHAR;
    ret.c = buf[0];
  } else if (buf[0] == '\x1b' && buf[1] == '[' && buf[2] == 'M') {
    ret.key_type = KEY_MOUSE;
    capture_mouse(buf, &ret);
  } else {
    ret.key_type = KEY_SPECIAL;
    if (buf[0] == '\x1b' && buf[1] == '[') {
      int using_modifiers = buf[3] == ';';
      int pos;
      if (!using_modifiers) {
        pos = 2;
        ret.special_modifier = 0;
      } else {
        pos = 4;
        switch (buf[3]) {
        case '2':
          ret.special_modifier = SHIFT;
          break;
        case '3':
          ret.special_modifier = ALT;
          break;
        case '5':
          ret.special_modifier = CNTRL;
          break;
        case '7':
          ret.special_modifier = CNTRL_ALT;
          break;
        default:
          ret.special_modifier = 0;
          break;
        }
      }
      switch (buf[pos]) {
      case 'A':
        ret.special_key = KEY_UP;
        break;
      case 'B':
        ret.special_key = KEY_DOWN;
        break;
      case 'C':
        ret.special_key = KEY_RIGHT;
        break;
      case 'D':
        ret.special_key = KEY_LEFT;
        break;
      case '3':
        ret.special_key = KEY_DELETE;
        break;
      default:
        ret.special_key = 99;
        break;
      }
    }
  }
  return ret;
}

KeyEvent read_key() {
  while (true) {
    KeyEvent ret = read_key_nonblock();
    if (ret.key_type != KEY_NONE)
      return ret;
  }
}

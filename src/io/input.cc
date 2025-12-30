#include "io/ui.h"

static Queue<char> input_queue;

int get_utf8_seq_len(uint8_t byte) {
  if ((byte & 0x80) == 0x00)
    return 1;
  if ((byte & 0xE0) == 0xC0)
    return 2;
  if ((byte & 0xF0) == 0xE0)
    return 3;
  if ((byte & 0xF8) == 0xF0)
    return 4;
  return 1;
}

int get_next_byte(char *out) {
  if (!input_queue.empty()) {
    input_queue.pop(*out);
    return 1;
  }
  int n = read(STDIN_FILENO, out, 1);
  return (n > 0) ? 1 : 0;
}

void enqueue_bytes(const char *bytes, int len) {
  for (int i = 0; i < len; i++)
    input_queue.push(bytes[i]);
}

int read_input(char *&buf) {
  size_t cap = 16;
  buf = (char *)calloc(cap, sizeof(char));
  size_t len = 0;
  char header;
  if (!get_next_byte(&header)) {
    free(buf);
    return 0;
  }
  if (header == '\x1b') {
    buf[len++] = header;
    while (len < 6) {
      char next_c;
      if (!get_next_byte(&next_c))
        break;
      buf[len++] = next_c;
    }
    return len;
  }
  int seq_len = get_utf8_seq_len((uint8_t)header);
  buf[len++] = header;
  if (seq_len == 1)
    return len;
  for (int i = 1; i < seq_len; i++) {
    char next_c;
    if (!get_next_byte(&next_c)) {
      enqueue_bytes(buf, len);
      free(buf);
      return 0;
    }
    buf[len++] = next_c;
  }
  uint_least32_t current_cp, prev_cp;
  grapheme_decode_utf8(buf, len, &prev_cp);
  uint_least16_t state = 0;
  while (true) {
    char next_header;
    if (!get_next_byte(&next_header))
      break;
    int next_seq_len = get_utf8_seq_len((uint8_t)next_header);
    char temp_seq[5];
    temp_seq[0] = next_header;
    int temp_len = 1;
    bool complete_seq = true;
    for (int i = 1; i < next_seq_len; i++) {
      char c;
      if (!get_next_byte(&c)) {
        complete_seq = false;
        break;
      }
      temp_seq[temp_len++] = c;
    }
    if (!complete_seq) {
      enqueue_bytes(temp_seq, temp_len);
      break;
    }
    grapheme_decode_utf8(temp_seq, temp_len, &current_cp);
    if (grapheme_is_character_break(prev_cp, current_cp, &state)) {
      enqueue_bytes(temp_seq, temp_len);
      break;
    } else {
      if (len + temp_len + 1 >= cap) {
        cap *= 2;
        buf = (char *)realloc(buf, cap);
      }
      memcpy(buf + len, temp_seq, temp_len);
      len += temp_len;
      prev_cp = current_cp;
    }
  }
  buf[len] = '\0';
  return len;
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

bool read_bracketed_paste(char **out, int *out_len) {
  size_t cap = 256, len = 0;
  char *buf = (char *)malloc(cap);
  if (!buf)
    return false;
  char window[6] = {0};
  size_t wlen = 0;
  auto push_byte = [&](char c) {
    if (len + 1 >= cap) {
      cap *= 2;
      buf = (char *)realloc(buf, cap);
    }
    buf[len++] = c;
  };
  while (true) {
    char c;
    if (!get_next_byte(&c)) {
      free(buf);
      return false;
    }
    if (wlen < 5) {
      window[wlen++] = c;
    } else {
      memmove(window, window + 1, 4);
      window[4] = c;
      wlen = 5;
    }
    if (wlen == 5 && window[0] == '\x1b' && window[1] == '[' &&
        window[2] == '2' && window[3] == '0' && window[4] == '1') {
      char tilde;
      if (!get_next_byte(&tilde)) {
        free(buf);
        return false;
      }
      if (tilde == '~')
        break;
      for (int i = 0; i < 5; i++)
        push_byte(window[i]);
      push_byte(tilde);
      wlen = 0;
      continue;
    }
    if (wlen == 5) {
      push_byte(window[0]);
      memmove(window, window + 1, 4);
      wlen = 4;
    }
  }
  buf[len] = '\0';
  *out = buf;
  *out_len = (int)len;
  return true;
}

KeyEvent read_key() {
  KeyEvent ret;
  char *buf;
  int n = read_input(buf);
  if (n <= 0) {
    ret.key_type = KEY_NONE;
    return ret;
  }
  if (n >= 6 && buf[0] == '\x1b' && buf[1] == '[' && buf[2] == '2' &&
      buf[3] == '0' && buf[4] == '0' && buf[5] == '~') {
    char *pbuf = nullptr;
    int plen = 0;
    if (read_bracketed_paste(&pbuf, &plen)) {
      free(buf);
      ret.key_type = KEY_PASTE;
      ret.c = pbuf;
      ret.len = plen;
      return ret;
    }
  } else if (n >= 3 && buf[0] == '\x1b' && buf[1] == '[' && buf[2] == 'M') {
    ret.key_type = KEY_MOUSE;
    capture_mouse(buf, &ret);
  } else if (n >= 2 && buf[0] == '\x1b' && buf[1] == '[') {
    ret.key_type = KEY_SPECIAL;
    int using_modifiers = buf[3] == ';';
    int pos;
    if (!using_modifiers) {
      pos = 2;
      ret.special_modifier = 0;
    } else {
      pos = 5;
      switch (buf[4]) {
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
  } else if (n > 0) {
    ret.key_type = KEY_CHAR;
    ret.c = buf;
    ret.len = n;
    return ret;
  }
  if (n > 0)
    free(buf);
  return ret;
}

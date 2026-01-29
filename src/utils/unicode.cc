#include "utils/utils.h"

int display_width(const char *str, size_t len) {
  if (!str || !*str)
    return 0;
  if (str[0] == '\t')
    return 4;
  unicode_width_state_t state;
  unicode_width_init(&state);
  int width = 0;
  for (size_t j = 0; j < len; j++) {
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

uint8_t utf8_codepoint_width(unsigned char c) {
  if ((c & 0x80) == 0x00)
    return 1;
  if ((c & 0xE0) == 0xC0)
    return 2;
  if ((c & 0xF0) == 0xE0)
    return 3;
  if ((c & 0xF8) == 0xF0)
    return 4;
  return 1;
}

uint32_t get_visual_col_from_bytes(const char *line, uint32_t len,
                                   uint32_t byte_limit) {
  if (!line)
    return 0;
  uint32_t visual_col = 0;
  uint32_t current_byte = 0;
  if (len > 0 && line[len - 1] == '\n')
    len--;
  while (current_byte < byte_limit && current_byte < len) {
    uint32_t inc = grapheme_next_character_break_utf8(line + current_byte,
                                                      len - current_byte);
    if (current_byte + inc > byte_limit)
      break;
    int w = display_width(line + current_byte, inc);
    if (w < 0)
      w = 0;
    visual_col += (uint32_t)w;
    current_byte += inc;
  }
  return visual_col;
}

uint32_t get_bytes_from_visual_col(const char *line, uint32_t len,
                                   uint32_t target_visual_col) {
  if (!line)
    return 0;
  uint32_t current_byte = 0;
  uint32_t visual_col = 0;
  if (len > 0 && line[len - 1] == '\n')
    len--;
  while (current_byte < len && visual_col < target_visual_col) {
    uint32_t inc = grapheme_next_character_break_utf8(line + current_byte,
                                                      len - current_byte);
    int w = display_width(line + current_byte, inc);
    if (w < 0)
      w = 0;
    if (visual_col + (uint32_t)w > target_visual_col)
      return current_byte;
    visual_col += (uint32_t)w;
    current_byte += inc;
  }
  return current_byte;
}

uint32_t count_clusters(const char *line, size_t len, size_t from, size_t to) {
  uint32_t count = 0;
  size_t pos = from;
  while (pos < to && pos < len) {
    size_t next =
        pos + grapheme_next_character_break_utf8(line + pos, len - pos);
    if (next > to)
      break;
    pos = next;
    count++;
  }
  return count;
}

size_t utf8_offset_to_utf16(const char *s, size_t utf8_len, size_t byte_pos) {
  if (byte_pos > utf8_len)
    return 0;
  size_t utf16_units = 0;
  size_t i = 0;
  while (i < byte_pos) {
    unsigned char c = s[i];
    if ((c & 0x80) == 0x00) {
      i += 1;
      utf16_units += 1;
    } else if ((c & 0xE0) == 0xC0) {
      i += 2;
      utf16_units += 1;
    } else if ((c & 0xF0) == 0xE0) {
      i += 3;
      utf16_units += 1;
    } else {
      i += 4;
      utf16_units += 2;
    }
  }
  return utf16_units;
}

size_t utf16_offset_to_utf8(const char *s, size_t utf8_len, size_t utf16_pos) {
  size_t utf16_units = 0;
  size_t i = 0;
  while (utf16_units < utf16_pos && i < utf8_len) {
    unsigned char c = s[i];
    if ((c & 0x80) == 0x00) {
      i += 1;
      utf16_units += 1;
    } else if ((c & 0xE0) == 0xC0) {
      i += 2;
      utf16_units += 1;
    } else if ((c & 0xF0) == 0xE0) {
      i += 3;
      utf16_units += 1;
    } else {
      i += 4;
      utf16_units += 2;
    }
  }
  return i;
}

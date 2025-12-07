extern "C" {
#include "../libs/libgrapheme/grapheme.h"
}
#include "../include/utils.h"
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <limits.h>
#include <string.h>
#include <string>
#include <unistd.h>

std::string get_exe_dir() {
  char exe_path[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", exe_path, PATH_MAX);
  if (count == -1)
    return "";
  exe_path[count] = '\0';
  std::string path(exe_path);
  return path.substr(0, path.find_last_of('/'));
}

uint32_t grapheme_strlen(const char *s) {
  if (!s)
    return 0;
  uint32_t count = 0;
  const char *p = s;
  while (*p) {
    uint32_t next = grapheme_next_character_break_utf8(p, UINT32_MAX);
    if (!next)
      next = 1;
    p += next;
    count++;
  }
  return count;
}

uint32_t get_visual_col_from_bytes(const char *line, uint32_t byte_limit) {
  if (!line)
    return 0;
  uint32_t visual_col = 0;
  uint32_t current_byte = 0;
  uint32_t len = strlen(line);
  if (len > 0 && line[len - 1] == '\n')
    len--;
  while (current_byte < byte_limit && current_byte < len) {
    uint32_t inc = grapheme_next_character_break_utf8(line + current_byte,
                                                      len - current_byte);
    if (current_byte + inc > byte_limit)
      break;
    current_byte += inc;
    visual_col++;
  }
  return visual_col;
}

uint32_t get_bytes_from_visual_col(const char *line,
                                   uint32_t target_visual_col) {
  if (!line)
    return 0;
  uint32_t visual_col = 0;
  uint32_t current_byte = 0;
  uint32_t len = strlen(line);
  if (len > 0 && line[len - 1] == '\n')
    len--;
  while (visual_col < target_visual_col && current_byte < len) {
    uint32_t inc = grapheme_next_character_break_utf8(line + current_byte,
                                                      len - current_byte);
    current_byte += inc;
    visual_col++;
  }
  return current_byte;
}

void log(const char *fmt, ...) {
  FILE *fp = fopen("/tmp/log.txt", "a");
  if (!fp)
    return;

  va_list args;
  va_start(args, fmt);
  vfprintf(fp, fmt, args);
  va_end(args);

  fputc('\n', fp);
  fclose(fp);
}

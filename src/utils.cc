extern "C" {
#include "../libs/libgrapheme/grapheme.h"
#include "../libs/unicode_width/unicode_width.h"
}
#include "../include/utils.h"
#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits.h>
#include <magic.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <unordered_map>

char *get_from_clipboard(uint32_t *out_len) {
  FILE *pipe = popen("xclip -selection clipboard -o", "r");
  if (!pipe) {
    *out_len = 0;
    return nullptr;
  }
  size_t capacity = 4096;
  size_t length = 0;
  char *buffer = (char *)malloc(capacity);
  if (!buffer) {
    pclose(pipe);
    *out_len = 0;
    return nullptr;
  }
  size_t n;
  while ((n = fread(buffer + length, 1, capacity - length, pipe)) > 0) {
    length += n;
    if (length == capacity) {
      capacity *= 2;
      char *tmp = (char *)realloc(buffer, capacity);
      if (!tmp) {
        free(buffer);
        pclose(pipe);
        *out_len = 0;
        return nullptr;
      }
      buffer = tmp;
    }
  }
  pclose(pipe);
  char *result = (char *)realloc(buffer, length + 1);
  if (result) {
    result[length] = '\0';
    buffer = result;
  } else {
    buffer[length] = '\0';
  }
  *out_len = length;
  return buffer;
}

void copy_to_clipboard(const char *text, size_t len) {
  FILE *pipe = popen("xclip -selection clipboard", "w");
  if (!pipe)
    return;
  fwrite(text, sizeof(char), len, pipe);
  pclose(pipe);
}

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

std::string get_exe_dir() {
  char exe_path[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", exe_path, PATH_MAX);
  if (count == -1)
    return "";
  exe_path[count] = '\0';
  std::string path(exe_path);
  return path.substr(0, path.find_last_of('/'));
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

char *load_file(const char *path, uint32_t *out_len) {
  std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
  if (!file.is_open())
    return nullptr;
  std::streamsize len = file.tellg();
  if (len < 0 || (std::uint32_t)len > 0xFFFFFFFF)
    return nullptr;
  file.seekg(0, std::ios::beg);
  char *buf = (char *)malloc(static_cast<std::uint32_t>(len));
  if (!buf)
    return nullptr;
  if (file.read(buf, len)) {
    *out_len = static_cast<uint32_t>(len);
    return buf;
  } else {
    free(buf);
    return nullptr;
  }
}

static std::string file_extension(const char *filename) {
  std::string name(filename);
  auto pos = name.find_last_of('.');
  if (pos == std::string::npos)
    return "";
  std::string ext = name.substr(pos + 1);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return ext;
}

char *detect_file_type(const char *filename) {
  magic_t magic = magic_open(MAGIC_MIME_TYPE);
  if (!magic)
    return nullptr;
  if (magic_load(magic, nullptr) != 0) {
    magic_close(magic);
    return nullptr;
  }
  const char *type = magic_file(magic, filename);
  if (!type) {
    magic_close(magic);
    return nullptr;
  }
  char *result = strdup(type);
  magic_close(magic);
  return result;
}

static const std::unordered_map<std::string, Language> ext_map = {
    {"sh", {"bash", tree_sitter_bash}},
    {"bash", {"bash", tree_sitter_bash}},
    {"c", {"c", tree_sitter_c}},
    {"cpp", {"cpp", tree_sitter_cpp}},
    {"cxx", {"cpp", tree_sitter_cpp}},
    {"cc", {"cpp", tree_sitter_cpp}},
    {"hpp", {"cpp", tree_sitter_cpp}},
    {"hh", {"cpp", tree_sitter_cpp}},
    {"hxx", {"cpp", tree_sitter_cpp}},
    {"h", {"cpp", tree_sitter_cpp}},
    {"css", {"css", tree_sitter_css}},
    {"fish", {"fish", tree_sitter_fish}},
    {"go", {"go", tree_sitter_go}},
    {"hs", {"haskell", tree_sitter_haskell}},
    {"html", {"html", tree_sitter_html}},
    {"htm", {"html", tree_sitter_html}},
    {"js", {"javascript", tree_sitter_javascript}},
    {"json", {"json", tree_sitter_json}},
    {"lua", {"lua", tree_sitter_lua}},
    {"mk", {"make", tree_sitter_make}},
    {"makefile", {"make", tree_sitter_make}},
    {"py", {"python", tree_sitter_python}},
    {"rb", {"ruby", tree_sitter_ruby}},
};

static const std::unordered_map<std::string, Language> mime_map = {
    {"text/x-c", {"c", tree_sitter_c}},
    {"text/x-c++", {"cpp", tree_sitter_cpp}},
    {"text/x-shellscript", {"bash", tree_sitter_bash}},
    {"application/json", {"json", tree_sitter_json}},
    {"text/javascript", {"javascript", tree_sitter_javascript}},
    {"text/html", {"html", tree_sitter_html}},
    {"text/css", {"css", tree_sitter_css}},
    {"text/x-python", {"python", tree_sitter_python}},
    {"text/x-ruby", {"ruby", tree_sitter_ruby}},
    {"text/x-go", {"go", tree_sitter_go}},
    {"text/x-haskell", {"haskell", tree_sitter_haskell}},
    {"text/x-lua", {"lua", tree_sitter_lua}},
};

Language language_for_file(const char *filename) {
  std::string ext = file_extension(filename);
  if (!ext.empty()) {
    auto it = ext_map.find(ext);
    if (it != ext_map.end())
      return it->second;
  }
  char *mime = detect_file_type(filename);
  if (mime) {
    std::string mime_type(mime);
    free(mime);
    auto it = mime_map.find(mime_type);
    if (it != mime_map.end())
      return it->second;
  }
  return {"unknown", nullptr};
}

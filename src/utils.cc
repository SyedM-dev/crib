extern "C" {
#include "../libs/libgrapheme/grapheme.h"
#include "../libs/unicode_width/unicode_width.h"
}
#include "../include/maps.h"
#include "../include/utils.h"

static std::string percent_encode(const std::string &s) {
  static const char *hex = "0123456789ABCDEF";
  std::string out;
  for (unsigned char c : s) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' ||
        c == '/') {
      out.push_back(c);
    } else {
      out.push_back('%');
      out.push_back(hex[c >> 4]);
      out.push_back(hex[c & 0xF]);
    }
  }
  return out;
}

std::string path_abs(const std::string &path_str) {
  namespace fs = std::filesystem;
  fs::path p = fs::weakly_canonical(fs::absolute(fs::path(path_str)));
  return p.generic_string();
}

std::string path_to_file_uri(const std::string &path_str) {
  return "file://" + percent_encode(path_abs(path_str));
}

uint64_t fnv1a_64(const char *s, size_t len) {
  uint64_t hash = 1469598103934665603ull;
  for (size_t i = 0; i < len; ++i) {
    hash ^= (uint8_t)s[i];
    hash *= 1099511628211ull;
  }
  return hash;
}

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
  if (pos == std::string::npos) {
    auto pos2 = name.find_last_of('/');
    if (pos2 != std::string::npos)
      pos = pos2;
    else
      return "";
  }
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

Language language_for_file(const char *filename) {
  std::string ext = file_extension(filename);
  std::string lang_name;
  if (!ext.empty()) {
    auto it = kExtToLang.find(ext);
    if (it != kExtToLang.end())
      return kLanguages.find(it->second)->second;
  }
  char *mime = detect_file_type(filename);
  if (mime) {
    std::string mime_type(mime);
    free(mime);
    auto it = kMimeToLang.find(mime_type);
    if (it != kMimeToLang.end())
      return kLanguages.find(it->second)->second;
  }
  return {"unknown", nullptr};
}

int utf8_byte_offset_to_utf16(const char *s, size_t byte_pos) {
  int utf16_units = 0;
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

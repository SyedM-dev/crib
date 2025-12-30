#include "config.h"
#include "utils/utils.h"

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

std::string path_abs(const std::string &path_str) {
  namespace fs = std::filesystem;
  fs::path p = fs::weakly_canonical(fs::absolute(fs::path(path_str)));
  return p.generic_string();
}

std::string path_to_file_uri(const std::string &path_str) {
  return "file://" + percent_encode(path_abs(path_str));
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

char *load_file(const char *path, uint32_t *out_len) {
  std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
  if (!file.is_open())
    return nullptr;
  std::streamsize len = file.tellg();
  if (len < 0 || static_cast<uint32_t>(len) > 0xFFFFFFFF)
    return nullptr;
  file.seekg(0, std::ios::beg);
  bool add_newline = false;
  if (len > 0) {
    file.seekg(-1, std::ios::end);
    char last_char;
    file.read(&last_char, 1);
    if (last_char != '\n')
      add_newline = true;
  }
  file.seekg(0, std::ios::beg);
  uint32_t alloc_size = static_cast<uint32_t>(len) + (add_newline ? 1 : 0);
  char *buf = (char *)malloc(alloc_size);
  if (!buf)
    return nullptr;
  if (!file.read(buf, len)) {
    free(buf);
    return nullptr;
  }
  if (add_newline)
    buf[len++] = '\n';
  *out_len = static_cast<uint32_t>(len);
  return buf;
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

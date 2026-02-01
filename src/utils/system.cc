#include "scripting/decl.h"
#include "utils/utils.h"

std::unordered_map<std::string, Language> languages;
std::unordered_map<std::string, std::string> language_extensions;
std::unordered_map<std::string, LSP> lsps;

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

std::string filename_from_path(const std::string &path) {
  auto pos = path.find_last_of('/');
  if (pos == std::string::npos)
    return path;
  return path.substr(pos + 1);
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

char *load_file(const char *path, uint32_t *out_len, bool *out_eol) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    return nullptr;
  std::streamsize len = file.tellg();
  if (len < 0 || static_cast<uint64_t>(len) > 0xFFFFFFFF)
    return nullptr;
  file.seekg(0, std::ios::beg);
  unsigned char bom[3] = {0};
  file.read(reinterpret_cast<char *>(bom), 3);
  if ((bom[0] == 0xFF && bom[1] == 0xFE) || (bom[0] == 0xFE && bom[1] == 0xFF))
    return nullptr;
  bool has_utf8_bom = (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF);
  uint32_t skip = has_utf8_bom ? 3 : 0;
  uint32_t data_len = static_cast<uint32_t>(len) - skip;
  file.seekg(skip, std::ios::beg);
  char *buf = (char *)malloc(data_len + 1);
  if (!buf)
    return nullptr;
  file.read(buf, data_len);
  bool has_cr = memchr(buf, '\r', data_len) != nullptr;
  bool has_lf = memchr(buf, '\n', data_len) != nullptr;
  if (!has_cr && !has_lf) {
    uint32_t write = data_len;
    buf[write++] = '\n';
    *out_len = write;
    return buf;
  }
  if (!has_cr) {
    *out_eol = true;
    uint32_t write = data_len;
    if (buf[write - 1] != '\n')
      buf[write++] = '\n';
    *out_len = write;
    return buf;
  }
  *out_eol = false;
  uint32_t write = 0;
  for (uint32_t i = 0; i < data_len; ++i)
    if (buf[i] != '\r')
      buf[write++] = buf[i];
  if (buf[write - 1] != '\n')
    buf[write++] = '\n';
  *out_len = write;
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

Language language_for_file(const char *filename) {
  std::string ext = file_extension(filename);
  if (!ext.empty()) {
    auto it = language_extensions.find(ext);
    if (it != language_extensions.end())
      return languages.find(it->second)->second;
  }
  std::string lang_name = ruby_file_detect(filename);
  if (!lang_name.empty()) {
    auto it = languages.find(lang_name);
    if (it != languages.end())
      return it->second;
  }
  return Language{};
}

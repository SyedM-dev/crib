#ifndef UTILS_H
#define UTILS_H

#include "pch.h"

template <typename T> struct Queue {
  std::queue<T> q;
  std::mutex m;

  void push(T val) {
    std::lock_guard<std::mutex> lock(m);
    q.push(val);
  }
  std::optional<T> front() {
    if (q.empty())
      return std::nullopt;
    return q.front();
  }
  bool pop(T &val) {
    std::lock_guard<std::mutex> lock(m);
    if (q.empty())
      return false;
    val = q.front();
    q.pop();
    return true;
  }
  void pop() {
    std::lock_guard<std::mutex> lock(m);
    q.pop();
  }
  bool empty() {
    std::lock_guard<std::mutex> lock(m);
    return q.empty();
  }
};

struct Coord {
  uint32_t row;
  uint32_t col;

  bool operator<(const Coord &other) const {
    return row < other.row || (row == other.row && col < other.col);
  }
  bool operator<=(const Coord &other) const {
    return *this < other || *this == other;
  }
  bool operator==(const Coord &other) const {
    return row == other.row && col == other.col;
  }
  bool operator!=(const Coord &other) const { return !(*this == other); }
  bool operator>(const Coord &other) const { return other < *this; }
  bool operator>=(const Coord &other) const { return !(*this < other); }
};

struct Match {
  size_t start;
  size_t end;
  std::string text;
};

struct Language {
  std::string name;
  uint8_t lsp_id;
  uint32_t color;
  const char *symbol;
};

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define UNUSED(x) (void)(x)
#define USING(x) UNUSED(sizeof(x))

inline uint32_t HEX(const std::string &s) {
  if (s.empty())
    return 0xFFFFFF;
  size_t start = (s.front() == '#') ? 1 : 0;
  return static_cast<uint32_t>(std::stoul(s.substr(start), nullptr, 16));
}

bool compare(const char *a, const char *b, size_t n);
std::string clean_text(const std::string &input);
std::string percent_encode(const std::string &s);
std::string percent_decode(const std::string &s);
uint32_t count_clusters(const char *line, size_t len, size_t from, size_t to);
std::string trim(const std::string &s);
std::string substitute_fence(const std::string &documentation,
                             const std::string &lang);

int display_width(const char *str, size_t len);
uint32_t get_visual_col_from_bytes(const char *line, uint32_t len,
                                   uint32_t byte_limit);
uint32_t get_bytes_from_visual_col(const char *line, uint32_t len,
                                   uint32_t target_visual_col);
size_t utf8_offset_to_utf16(const char *utf8, size_t utf8_len,
                            size_t byte_offset);
size_t utf16_offset_to_utf8(const char *utf8, size_t utf8_len,
                            size_t utf16_offset);
uint8_t utf8_codepoint_width(unsigned char c);

void log(const char *fmt, ...);

std::string path_abs(const std::string &path_str);
std::string path_to_file_uri(const std::string &path_str);
std::string filename_from_path(const std::string &path);
std::string get_exe_dir();
char *load_file(const char *path, uint32_t *out_len);
char *detect_file_type(const char *filename);
Language language_for_file(const char *filename);

void copy_to_clipboard(const char *text, size_t len);
char *get_from_clipboard(uint32_t *out_len);

template <typename T>
inline T *safe_get(std::map<uint16_t, T> &m, uint16_t key) {
  auto it = m.find(key);
  if (it == m.end())
    return nullptr;
  return &it->second;
}

template <typename Func, typename... Args>
auto throttle(std::chrono::milliseconds min_duration, Func &&func,
              Args &&...args) {
  auto start = std::chrono::steady_clock::now();
  if constexpr (std::is_void_v<std::invoke_result_t<Func, Args...>>) {
    std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
  } else {
    auto result =
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    auto elapsed = std::chrono::steady_clock::now() - start;
    if (elapsed < min_duration)
      std::this_thread::sleep_for(min_duration - elapsed);
    return result;
  }
  auto elapsed = std::chrono::steady_clock::now() - start;
  if (elapsed < min_duration)
    std::this_thread::sleep_for(min_duration - elapsed);
}

#endif

#ifndef UTILS_H
#define UTILS_H

#include "./pch.h"
#include "./ts_def.h"

template <typename T> struct Queue {
  std::queue<T> q;
  std::mutex m;

  void push(T val) {
    std::lock_guard<std::mutex> lock(m);
    q.push(val);
  }
  T front() {
    std::lock_guard<std::mutex> lock(m);
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

std::string path_to_file_uri(const std::string &path_str);
int display_width(const char *str, size_t len);
uint32_t get_visual_col_from_bytes(const char *line, uint32_t len,
                                   uint32_t byte_limit);
uint32_t get_bytes_from_visual_col(const char *line, uint32_t len,
                                   uint32_t target_visual_col);
void log(const char *fmt, ...);
std::string get_exe_dir();
char *load_file(const char *path, uint32_t *out_len);
char *detect_file_type(const char *filename);
int utf8_byte_offset_to_utf16(const char *s, size_t byte_pos);
Language language_for_file(const char *filename);
void copy_to_clipboard(const char *text, size_t len);
char *get_from_clipboard(uint32_t *out_len);
uint32_t count_clusters(const char *line, size_t len, size_t from, size_t to);

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

#ifndef UTILS_H
#define UTILS_H

#include "./ts_def.h"
#include <mutex>
#include <queue>
#include <string>

template <typename T> struct Queue {
  std::queue<T> q;
  std::mutex m;

  void push(T val) {
    std::lock_guard<std::mutex> lock(m);
    q.push(val);
  }
  bool pop(T &val) {
    std::lock_guard<std::mutex> lock(m);
    if (q.empty())
      return false;
    val = q.front();
    q.pop();
    return true;
  }
  bool empty() {
    std::lock_guard<std::mutex> lock(m);
    return q.empty();
  }
};

uint32_t grapheme_strlen(const char *s);
uint32_t get_visual_col_from_bytes(const char *line, uint32_t byte_limit);
uint32_t get_bytes_from_visual_col(const char *line,
                                   uint32_t target_visual_col);
void log(const char *fmt, ...);
std::string get_exe_dir();
char *load_file(const char *path, uint32_t *out_len);
char *detect_file_type(const char *filename);
Language language_for_file(const char *filename);

#endif

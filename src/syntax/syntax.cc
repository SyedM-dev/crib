#include "io/knot.h"
#include "main.h"
#include "syntax/langs.h"
#include "syntax/parser.h"

Parser::Parser(Knot *n_root, std::shared_mutex *n_knot_mutex,
               std::string n_lang, uint32_t n_scroll_max) {
  scroll_max = n_scroll_max;
  line_data.reserve(n_root->line_count + 1);
  knot_mutex = n_knot_mutex;
  lang = n_lang;
  auto pair = parsers.find(n_lang);
  if (pair != parsers.end()) {
    parse_func = std::get<0>(pair->second);
    state_match_func = std::get<1>(pair->second);
  } else {
    assert("unknown lang should be checked by caller" && 0);
  }
  edit(n_root, 0, 0, n_root->line_count);
}

void Parser::edit(Knot *n_root, uint32_t start_line, uint32_t old_end_line,
                  uint32_t new_end_line) {
  std::lock_guard lock(data_mutex);
  root = n_root;
  if (((int64_t)old_end_line - (int64_t)start_line) > 0)
    line_data.erase(line_data.begin() + start_line,
                    line_data.begin() + start_line + old_end_line - start_line);
  if (((int64_t)new_end_line - (int64_t)old_end_line) > 0)
    line_data.insert(line_data.begin() + start_line,
                     new_end_line - old_end_line, LineData{});
  dirty_lines.insert(start_line);
}

void Parser::work() {
  std::shared_lock k_lock(*knot_mutex);
  k_lock.unlock();
  uint32_t capacity = 256;
  char *text = (char *)calloc((capacity + 1), sizeof(char));
  std::set<uint32_t> tmp_dirty;
  std::unique_lock lock_data(data_mutex);
  tmp_dirty.swap(dirty_lines);
  lock_data.unlock();
  std::set<uint32_t> remaining_dirty;
  for (uint32_t c_line : tmp_dirty) {
    if (c_line > scroll_max) {
      remaining_dirty.insert(c_line);
      continue;
    }
    std::unique_lock lock(mutex);
    uint32_t line_count = (uint32_t)line_data.size();
    std::shared_ptr<void> prev_state =
        (c_line > 0) ? line_data[c_line - 1].out_state : nullptr;
    lock.unlock();
    while (c_line < line_count) {
      if (!running.load(std::memory_order_relaxed)) {
        free(text);
        return;
      }
      k_lock.lock();
      uint32_t r_offset, r_len;
      r_offset = line_to_byte(root, c_line, &r_len);
      if (r_len > capacity) {
        capacity = r_len;
        text = (char *)realloc(text, capacity + 1);
        memset(text, 0, capacity + 1);
      }
      read_into(root, r_offset, r_len, text);
      k_lock.unlock();
      if (c_line < scroll_max &&
          ((scroll_max > 100 && c_line > scroll_max - 100) || c_line < 100))
        lock.lock();
      lock_data.lock();
      std::shared_ptr<void> new_state =
          parse_func(&line_data[c_line].tokens, prev_state, text, r_len);
      lock_data.unlock();
      line_data[c_line].in_state = prev_state;
      line_data[c_line].out_state = new_state;
      if (lock.owns_lock())
        lock.unlock();
      if (!running.load(std::memory_order_relaxed)) {
        free(text);
        return;
      }
      prev_state = new_state;
      c_line++;
      if (c_line < line_count && c_line > scroll_max + 50) {
        if (c_line > 0)
          remaining_dirty.insert(c_line - 1);
        remaining_dirty.insert(c_line);
        break;
      }
      lock.lock();
      if (c_line < line_count &&
          state_match_func(prev_state, line_data[c_line].in_state))
        break;
      lock.unlock();
    }
    if (!running.load(std::memory_order_relaxed)) {
      free(text);
      return;
    }
  }
  free(text);
  lock_data.lock();
  dirty_lines = std::move(remaining_dirty);
}

void Parser::scroll(uint32_t line) {
  if (line != scroll_max) {
    scroll_max = line;
    uint32_t c_line = line > 100 ? line - 100 : 0;
    if (line_data.size() < c_line)
      return;
    if (line_data[c_line].in_state || line_data[c_line].out_state)
      return;
    std::shared_lock k_lock(*knot_mutex);
    k_lock.unlock();
    uint32_t capacity = 256;
    char *text = (char *)calloc((capacity + 1), sizeof(char));
    std::unique_lock lock_data(data_mutex);
    lock_data.unlock();
    std::unique_lock lock(mutex);
    uint32_t line_count = (uint32_t)line_data.size();
    std::shared_ptr<void> prev_state =
        (c_line > 0) ? line_data[c_line - 1].out_state : nullptr;
    lock.unlock();
    while (c_line < line_count) {
      if (!running.load(std::memory_order_relaxed)) {
        free(text);
        return;
      }
      k_lock.lock();
      uint32_t r_offset, r_len;
      r_offset = line_to_byte(root, c_line, &r_len);
      if (r_len > capacity) {
        capacity = r_len;
        text = (char *)realloc(text, capacity + 1);
        memset(text, 0, capacity + 1);
      }
      read_into(root, r_offset, r_len, text);
      k_lock.unlock();
      if (c_line < scroll_max &&
          ((scroll_max > 100 && c_line > scroll_max - 100) || c_line < 100))
        lock.lock();
      lock_data.lock();
      std::shared_ptr<void> new_state =
          parse_func(&line_data[c_line].tokens, prev_state, text, r_len);
      lock_data.unlock();
      line_data[c_line].in_state = nullptr;
      line_data[c_line].out_state = new_state;
      if (lock.owns_lock())
        lock.unlock();
      if (!running.load(std::memory_order_relaxed)) {
        free(text);
        return;
      }
      prev_state = new_state;
      c_line++;
      if (c_line < line_count && c_line > scroll_max + 50)
        break;
    }
    free(text);
  } else {
    scroll_max = line;
  }
}

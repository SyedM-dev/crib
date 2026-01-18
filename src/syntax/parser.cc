#include "syntax/parser.h"
#include "io/knot.h"
#include "main.h"
#include "syntax/decl.h"
#include "syntax/langs.h"

std::array<Highlight, TOKEN_KIND_COUNT> highlights = {};

Parser::Parser(Knot *n_root, std::shared_mutex *n_knot_mutex,
               std::string n_lang, uint32_t n_scroll_max) {
  scroll_max = n_scroll_max;
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
    line_tree.erase(start_line + 1, old_end_line - start_line);
  if (((int64_t)new_end_line - (int64_t)old_end_line) > 0)
    line_tree.insert(start_line + 1, new_end_line - start_line);
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
  std::unique_lock lock(mutex);
  lock.unlock();
  for (uint32_t c_line : tmp_dirty) {
    if (c_line > scroll_max) {
      remaining_dirty.insert(c_line);
      continue;
    }
    uint32_t line_count = line_tree.count();
    lock_data.lock();
    std::shared_ptr<void> prev_state =
        (c_line > 0) ? line_tree.at(c_line - 1)->out_state : nullptr;
    lock_data.unlock();
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
      if (line_tree.count() < c_line) {
        if (lock.owns_lock())
          lock.unlock();
        continue;
      }
      lock_data.lock();
      LineData *line_data = line_tree.at(c_line);
      std::shared_ptr<void> new_state =
          parse_func(&line_data->tokens, prev_state, text, r_len);
      line_data->in_state = prev_state;
      line_data->out_state = new_state;
      if (!running.load(std::memory_order_relaxed)) {
        free(text);
        return;
      }
      prev_state = new_state;
      c_line++;
      if (c_line < line_count && c_line > scroll_max + 50) {
        lock_data.unlock();
        if (lock.owns_lock())
          lock.unlock();
        if (c_line > 0)
          remaining_dirty.insert(c_line - 1);
        remaining_dirty.insert(c_line);
        break;
      }
      if (c_line < line_count &&
          state_match_func(prev_state, line_tree.at(c_line)->in_state)) {
        lock_data.unlock();
        if (lock.owns_lock())
          lock.unlock();
        break;
      }
      lock_data.unlock();
      if (lock.owns_lock())
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
    if (line_tree.count() < c_line)
      return;
    std::unique_lock lock_data(data_mutex);
    if (line_tree.at(c_line)->in_state || line_tree.at(c_line)->out_state)
      return;
    lock_data.unlock();
    std::shared_lock k_lock(*knot_mutex);
    k_lock.unlock();
    uint32_t capacity = 256;
    char *text = (char *)calloc((capacity + 1), sizeof(char));
    uint32_t line_count = line_tree.count();
    std::unique_lock lock(mutex);
    std::shared_ptr<void> prev_state =
        (c_line > 0) ? line_tree.at(c_line - 1)->out_state : nullptr;
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
      if (line_tree.count() < c_line) {
        if (lock.owns_lock())
          lock.unlock();
        continue;
      }
      lock_data.lock();
      LineData *line_data = line_tree.at(c_line);
      std::shared_ptr<void> new_state =
          parse_func(&line_data->tokens, prev_state, text, r_len);
      line_data->in_state = nullptr;
      line_data->out_state = new_state;
      lock_data.unlock();
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

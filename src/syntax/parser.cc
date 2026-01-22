#include "syntax/parser.h"
#include "editor/editor.h"
#include "io/knot.h"
#include "main.h"
#include "ruby/internal/special_consts.h"
#include "syntax/decl.h"
#include "syntax/langs.h"

std::array<Highlight, TOKEN_KIND_COUNT> highlights = {};

Parser::Parser(Editor *n_editor, std::string n_lang, uint32_t n_scroll_max) {
  editor = n_editor;
  scroll_max = n_scroll_max;
  lang = n_lang;
  auto custom_parser = custom_highlighters.find(n_lang);
  if (custom_parser != custom_highlighters.end()) {
    parser_block = custom_parser->second.first;
    match_block = custom_parser->second.second;
    is_custom = true;
  } else {
    auto pair = parsers.find(n_lang);
    if (pair != parsers.end()) {
      parse_func = std::get<0>(pair->second);
      state_match_func = std::get<1>(pair->second);
      is_custom = false;
    } else {
      assert("unknown lang should be checked by caller" && 0);
    }
  }
  edit(0, 0, editor->root->line_count);
}

void Parser::edit(uint32_t start_line, uint32_t old_end_line,
                  uint32_t inserted_rows) {
  std::lock_guard lock(data_mutex);
  if (((int64_t)old_end_line - (int64_t)start_line) > 0)
    line_tree.erase(start_line, old_end_line - start_line);
  if (inserted_rows > 0)
    line_tree.insert(start_line, inserted_rows);
  if (start_line > 0)
    dirty_lines.push(start_line - 1);
  dirty_lines.push(start_line);
  dirty_lines.push(start_line + 1);
}

void Parser::work() {
  if (!editor || !editor->root)
    return;
  std::shared_lock k_lock(editor->knot_mtx);
  k_lock.unlock();
  uint32_t capacity = 256;
  char *text = (char *)calloc((capacity + 1), sizeof(char));
  std::unique_lock lock_data(data_mutex);
  lock_data.unlock();
  std::unique_lock lock(mutex);
  lock.unlock();
  uint32_t c_line;
  while (dirty_lines.pop(c_line)) {
    if (!running.load(std::memory_order_relaxed)) {
      free(text);
      return;
    }
    if (c_line > scroll_max + 40) {
      dirty_lines.push(c_line);
      continue;
    }
    uint32_t line_count = line_tree.count();
    lock_data.lock();
    std::shared_ptr<void> prev_state =
        (c_line > 0) && c_line < line_tree.count()
            ? line_tree.at(c_line - 1)->out_state
            : nullptr;
    lock_data.unlock();
    while (c_line < line_count) {
      if (!running.load(std::memory_order_relaxed)) {
        free(text);
        return;
      }
      if (scroll_dirty.exchange(false, std::memory_order_acq_rel)) {
        dirty_lines.push(c_line);
        c_line = scroll_max < 50 ? 0 : scroll_max - 50;
      }
      k_lock.lock();
      if (c_line > editor->root->line_count) {
        k_lock.unlock();
        continue;
      }
      uint32_t r_offset, r_len;
      r_offset = line_to_byte(editor->root, c_line, &r_len);
      if (r_len > capacity) {
        capacity = r_len;
        text = (char *)realloc(text, capacity + 1);
        memset(text, 0, capacity + 1);
      }
      read_into(editor->root, r_offset, r_len, text);
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
      if (!line_data) {
        lock_data.unlock();
        if (lock.owns_lock())
          lock.unlock();
        continue;
      }
      std::shared_ptr<void> new_state{nullptr};
      if (is_custom) {
        VALUE state = Qnil;
        if (prev_state) {
          std::shared_ptr<CustomState> state_ptr =
              std::static_pointer_cast<CustomState>(prev_state);
          state = state_ptr->state;
        }
        VALUE out_state =
            parse_custom(&line_data->tokens, parser_block, text, r_len, state);
        std::shared_ptr<CustomState> out_state_ptr =
            std::make_shared<CustomState>(out_state);
        new_state = out_state_ptr;
      } else {
        new_state = parse_func(&line_data->tokens, prev_state, text, r_len);
      }
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
          dirty_lines.push(c_line - 1);
        dirty_lines.push(c_line);
        break;
      }
      if (c_line < line_count && (line_data = line_tree.at(c_line))) {
        bool done = false;
        if (is_custom) {
          VALUE in_state_v = Qnil;
          if (prev_state)
            in_state_v =
                std::static_pointer_cast<CustomState>(prev_state)->state;
          VALUE out_state_v = Qnil;
          if (line_data->in_state)
            out_state_v =
                std::static_pointer_cast<CustomState>(line_data->in_state)
                    ->state;
          done = custom_compare(match_block, in_state_v, out_state_v);
        } else {
          done = state_match_func(prev_state, line_data->in_state);
        }
        if (done) {
          lock_data.unlock();
          if (lock.owns_lock())
            lock.unlock();
          break;
        }
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
}

void Parser::scroll(uint32_t line) {
  if (line != scroll_max) {
    scroll_max = line;
    uint32_t c_line = line > 50 ? line - 50 : 0;
    if (c_line >= line_tree.count())
      return;
    std::unique_lock lock_data(data_mutex);
    if (line_tree.at(c_line)->in_state || line_tree.at(c_line)->out_state)
      return;
    scroll_dirty = true;
    dirty_lines.push(c_line);
  } else {
    scroll_max = line;
  }
}

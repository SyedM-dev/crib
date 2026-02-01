#include "syntax/parser.h"
#include "editor/editor.h"
#include "io/knot.h"
#include "main.h"
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
  edit(0, 0, editor->root->line_count + 1);
}

void Parser::edit(uint32_t start_line, uint32_t removed_rows,
                  uint32_t inserted_rows) {
  int64_t delta = (int64_t)inserted_rows - (int64_t)removed_rows;
  if (delta < 0)
    line_tree.erase(start_line, (uint32_t)(-delta));
  else if (delta > 0)
    line_tree.insert(start_line, (uint32_t)delta);
  uint32_t span = MAX(removed_rows, inserted_rows);
  uint32_t begin = (start_line > 0) ? start_line - 1 : 0;
  uint32_t end = start_line + span;
  for (uint32_t line = begin; line <= end + 1; ++line)
    dirty_lines.push(line);
}

void Parser::work() {
  if (!editor || !editor->root)
    return;
  std::vector<uint32_t> batch;
  uint32_t c_line;
  while (dirty_lines.pop(c_line))
    batch.push_back(c_line);
  uint32_t i = MAX(0, (int64_t)scroll_max - 60);
  LineData *l_iter = line_tree.start_iter(i);
  while (l_iter && i < scroll_max + 10) {
    if (!l_iter->out_state)
      batch.push_back(i);
    i++;
    l_iter = line_tree.next();
  }
  line_tree.end_iter();
  for (uint32_t c_line : batch) {
    if (!running.load(std::memory_order_relaxed))
      break;
    uint32_t min_line = scroll_max > 60 ? scroll_max - 60 : 0;
    uint32_t max_line = scroll_max + 10;
    if (c_line < min_line || c_line > max_line) {
      dirty_lines.push(c_line);
      continue;
    }
    uint32_t scroll_snapshot = scroll_max;
    std::shared_ptr<void> prev_state = nullptr;
    uint32_t line_count;
    line_count = line_tree.count();
    if (c_line > 0 && c_line < line_count)
      prev_state = line_tree.at(c_line - 1)->out_state;
    std::shared_lock k_lock(editor->knot_mtx);
    LineIterator *it = begin_l_iter(editor->root, c_line);
    if (!it)
      continue;
    uint32_t cur_line = c_line;
    while (cur_line < line_count) {
      if (!running.load(std::memory_order_relaxed))
        break;
      if (scroll_snapshot != scroll_max) {
        LineData *line_data = line_tree.at(cur_line);
        if (line_data && !line_data->out_state)
          dirty_lines.push(cur_line);
        break;
      }
      if (cur_line < min_line || cur_line > max_line) {
        dirty_lines.push(cur_line);
        break;
      }
      uint32_t len;
      char *line = next_line(it, &len);
      if (!line)
        break;
      LineData *line_data = line_tree.at(cur_line);
      if (!line_data) {
        cur_line++;
        continue;
      }
      std::shared_ptr<void> new_state;
      if (is_custom) {
        std::string state = "";
        if (prev_state)
          state = std::static_pointer_cast<std::string>(prev_state)->c_str();
        std::string out_state = parse_custom(&line_data->tokens, parser_block,
                                             line, len, state, cur_line);
        new_state = std::make_shared<std::string>(out_state);
      } else {
        new_state =
            parse_func(&line_data->tokens, prev_state, line, len, cur_line);
      }
      line_data->in_state = prev_state;
      line_data->out_state = new_state;
      bool done = false;
      if (cur_line + 1 < line_count) {
        LineData *next_line_data = line_tree.at(cur_line + 1);
        if (next_line_data) {
          if (is_custom) {
            std::string a =
                prev_state
                    ? std::static_pointer_cast<std::string>(new_state)->c_str()
                    : "";
            std::string b = next_line_data->in_state
                                ? std::static_pointer_cast<std::string>(
                                      next_line_data->in_state)
                                      ->c_str()
                                : "";
            done = custom_compare(match_block, a, b);
          } else {
            done = state_match_func(new_state, next_line_data->in_state);
          }
        }
      }
      prev_state = new_state;
      cur_line++;
      if (done)
        break;
    }
    free(it->buffer);
    free(it);
  }
}

void Parser::scroll(uint32_t line) { scroll_max = line; }

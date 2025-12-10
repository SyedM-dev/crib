#include "../include/ts.h"
#include "../include/editor.h"
#include "../include/rope.h"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>

static std::unordered_map<std::string, std::regex> regex_cache;

static const std::regex scm_regex(
    R"((@[A-Za-z0-9_.]+)|(;; \#[0-9a-fA-F]{6} \#[0-9a-fA-F]{6} [01] [01] [01] \d+))");

TSQuery *load_query(const char *query_path, Editor *editor) {
  const TSLanguage *lang = editor->language;
  std::ifstream file(query_path, std::ios::in | std::ios::binary);
  if (!file.is_open())
    return nullptr;
  std::string highlight_query((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  std::smatch match;
  std::map<std::string, int> capture_name_cache;
  Highlight *c_hl = nullptr;
  int i = 0;
  int limit = 20;
  editor->query_map.resize(limit);
  std::string::const_iterator searchStart(highlight_query.cbegin());
  while (std::regex_search(searchStart, highlight_query.cend(), match,
                           scm_regex)) {
    std::string mct = match.str();
    if (mct.substr(0, 1) == "@") {
      std::string capture_name = mct;
      if (!capture_name_cache.count(capture_name)) {
        if (c_hl) {
          if (i >= limit) {
            limit += 20;
            editor->query_map.resize(limit);
          }
          editor->query_map[i] = *c_hl;
          delete c_hl;
          c_hl = nullptr;
        }
        capture_name_cache[capture_name] = i;
        i++;
      }
    } else if (mct.substr(0, 2) == ";;") {
      if (c_hl)
        delete c_hl;
      c_hl = new Highlight();
      c_hl->fg = HEX(mct.substr(4, 6));
      c_hl->bg = HEX(mct.substr(12, 6));
      int bold = std::stoi(mct.substr(19, 1));
      int italic = std::stoi(mct.substr(21, 1));
      int underline = std::stoi(mct.substr(23, 1));
      c_hl->priority = std::stoi(mct.substr(25));
      c_hl->flags = (bold ? CF_BOLD : 0) | (italic ? CF_ITALIC : 0) |
                    (underline ? CF_UNDERLINE : 0);
    }
    searchStart = match.suffix().first;
  }
  if (c_hl)
    delete c_hl;
  uint32_t error_offset = 0;
  TSQueryError error_type = (TSQueryError)0;
  TSQuery *q = ts_query_new(lang, highlight_query.c_str(),
                            (uint32_t)highlight_query.length(), &error_offset,
                            &error_type);
  return q;
}

static inline const TSNode *find_capture_node(const TSQueryMatch &match,
                                              uint32_t capture_id) {
  for (uint32_t i = 0; i < match.capture_count; i++)
    if (match.captures[i].index == capture_id)
      return &match.captures[i].node;
  return nullptr;
}

static inline std::string node_text(const TSNode &node, Knot *source) {
  uint32_t start = ts_node_start_byte(node);
  uint32_t end = ts_node_end_byte(node);
  char *text = read(source, start, end - start);
  std::string final = std::string(text, end - start);
  free(text);
  return final;
}

static inline bool ts_predicate(TSQuery *query, const TSQueryMatch &match,
                                Knot *source) {
  uint32_t step_count;
  const TSQueryPredicateStep *steps =
      ts_query_predicates_for_pattern(query, match.pattern_index, &step_count);
  if (!steps || step_count != 4)
    return true;
  if (source->char_count >= (16 * 1024))
    return false;
  std::string command;
  std::string regex_txt;
  uint32_t subject_id = 0;
  for (uint32_t i = 0; i < step_count; i++) {
    const TSQueryPredicateStep *step = &steps[i];
    if (step->type == TSQueryPredicateStepTypeDone)
      break;
    switch (step->type) {
    case TSQueryPredicateStepTypeString: {
      uint32_t length = 0;
      const char *s =
          ts_query_string_value_for_id(query, step->value_id, &length);
      if (i == 0)
        command.assign(s, length);
      else
        regex_txt.assign(s, length);
      break;
    }
    case TSQueryPredicateStepTypeCapture: {
      subject_id = step->value_id;
      break;
    }
    case TSQueryPredicateStepTypeDone:
      break;
    }
  }
  const TSNode *node = find_capture_node(match, subject_id);
  std::string subject = node_text(*node, source);
  auto it = regex_cache.find(regex_txt);
  if (it == regex_cache.end())
    it = regex_cache.emplace(regex_txt, std::regex(regex_txt)).first;
  const std::regex &re = it->second;
  if (command == "match?")
    return std::regex_match(subject, re);
  else if (command == "not-match?")
    return !std::regex_match(subject, re);
  return false;
}

const char *read_ts(void *payload, uint32_t byte_index, TSPoint,
                    uint32_t *bytes_read) {
  if (!running) {
    *bytes_read = 0;
    return "";
  }
  Editor *editor = (Editor *)payload;
  if (byte_index >= editor->root->char_count) {
    *bytes_read = 0;
    return "";
  }
  return leaf_from_offset(editor->root, byte_index, bytes_read);
}

static inline Highlight *safe_get(std::vector<Highlight> &vec, size_t index) {
  if (index >= vec.size())
    return nullptr;
  return &vec[index];
}

void ts_collect_spans(Editor *editor) {
  if (!editor->parser || !editor->root || !editor->query)
    return;
  TSInput tsinput = {
      .payload = editor,
      .read = read_ts,
      .encoding = TSInputEncodingUTF8,
      .decode = nullptr,
  };
  TSTree *tree, *copy = nullptr;
  std::unique_lock knot_mtx(editor->knot_mtx);
  if (editor->tree)
    copy = ts_tree_copy(editor->tree);
  knot_mtx.unlock();
  if (!running)
    return;
  std::vector<TSInputEdit> edits;
  TSInputEdit edit;
  if (copy)
    while (editor->edit_queue.pop(edit)) {
      edits.push_back(edit);
      ts_tree_edit(copy, &edits.back());
    };
  if (copy && edits.empty()) {
    ts_tree_delete(copy);
    return;
  }
  editor->spans.mid_parse = true;
  std::shared_lock lock(editor->knot_mtx);
  tree = ts_parser_parse(editor->parser, copy, tsinput);
  lock.unlock();
  if (copy)
    ts_tree_delete(copy);
  knot_mtx.lock();
  if (editor->tree)
    ts_tree_delete(editor->tree);
  editor->tree = tree;
  copy = ts_tree_copy(tree);
  knot_mtx.unlock();
  TSQueryCursor *cursor = ts_query_cursor_new();
  ts_query_cursor_exec(cursor, editor->query, ts_tree_root_node(copy));
  std::vector<Span> new_spans;
  new_spans.reserve(4096);
  TSQueryMatch match;
  while (ts_query_cursor_next_match(cursor, &match)) {
    if (!running)
      break;
    if (!ts_predicate(editor->query, match, editor->root))
      continue;
    for (uint32_t i = 0; i < match.capture_count; i++) {
      if (!running)
        break;
      TSQueryCapture cap = match.captures[i];
      uint32_t start = ts_node_start_byte(cap.node);
      uint32_t end = ts_node_end_byte(cap.node);
      Highlight *hl = safe_get(editor->query_map, cap.index);
      if (hl)
        new_spans.push_back({start, end, hl});
    }
  }
  ts_query_cursor_delete(cursor);
  ts_tree_delete(copy);
  if (!running)
    return;
  std::sort(new_spans.begin(), new_spans.end());
  std::pair<uint32_t, int64_t> span_edit;
  while (editor->spans.edits.pop(span_edit))
    apply_edit(new_spans, span_edit.first, span_edit.second);
  std::unique_lock span_mtx(editor->spans.mtx);
  editor->spans.mid_parse = false;
  editor->spans.spans.swap(new_spans);
  span_mtx.unlock();
}

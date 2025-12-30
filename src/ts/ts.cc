#include "ts/ts.h"
#include "editor/editor.h"
#include "io/knot.h"

const char *read_ts(void *payload, uint32_t byte_index, TSPoint,
                    uint32_t *bytes_read) {
  Editor *editor = (Editor *)payload;
  if (byte_index >= editor->root->char_count) {
    *bytes_read = 0;
    return "";
  }
  return leaf_from_offset(editor->root, byte_index, bytes_read);
}

void ts_collect_spans(Editor *editor) {
  static int parse_counter = 0;
  if (!editor->ts.parser || !editor->root || !editor->ts.query)
    return;
  const bool injections_enabled = editor->root->char_count < (1024 * 32);
  for (auto &inj : editor->ts.injections)
    inj.second.ranges.clear();
  TSInput tsinput{
      .payload = editor,
      .read = read_ts,
      .encoding = TSInputEncodingUTF8,
      .decode = nullptr,
  };
  std::vector<TSInputEdit> edits;
  TSInputEdit edit;
  if (!editor->edit_queue.empty()) {
    while (editor->edit_queue.pop(edit))
      edits.push_back(edit);
    if (editor->ts.tree) {
      for (auto &e : edits)
        ts_tree_edit(editor->ts.tree, &e);
    }
    for (auto &inj : editor->ts.injections) {
      if (inj.second.tree) {
        for (auto &e : edits) {
          TSInputEdit inj_edit = e;
          for (auto &r : inj.second.ranges) {
            if (e.start_byte >= r.start_byte && e.start_byte <= r.end_byte) {
              inj_edit.start_byte -= r.start_byte;
              inj_edit.old_end_byte -= r.start_byte;
              inj_edit.new_end_byte -= r.start_byte;
            }
          }
          ts_tree_edit(inj.second.tree, &inj_edit);
        }
      }
    }
  } else if (editor->ts.tree && parse_counter < 64) {
    parse_counter++;
    return;
  }
  parse_counter = 0;
  editor->spans.mid_parse = true;
  std::shared_lock lock(editor->knot_mtx);
  TSTree *tree = ts_parser_parse(editor->ts.parser, editor->ts.tree, tsinput);
  if (!tree)
    return;
  if (editor->ts.tree)
    ts_tree_delete(editor->ts.tree);
  editor->ts.tree = tree;
  lock.unlock();
  std::vector<Span> new_spans;
  new_spans.reserve(4096);
  struct PendingRanges {
    std::vector<TSRange> ranges;
    TSSet *tsset = nullptr;
  };
  struct WorkItem {
    TSSetBase *tsset;
    TSTree *tree;
    int depth;
  };
  const int kMaxInjectionDepth = 4;
  std::vector<WorkItem> work;
  work.push_back(
      {reinterpret_cast<TSSetBase *>(&editor->ts), editor->ts.tree, 0});
  auto overlaps = [](const Span &s, const TSRange &r) {
    return !(s.end <= r.start_byte || s.start >= r.end_byte);
  };
  auto remove_overlapping_spans = [&](const std::vector<TSRange> &ranges) {
    if (ranges.empty())
      return;
    new_spans.erase(
        std::remove_if(new_spans.begin(), new_spans.end(),
                       [&](const Span &sp) {
                         return std::any_of(
                             ranges.begin(), ranges.end(),
                             [&](const TSRange &r) { return overlaps(sp, r); });
                       }),
        new_spans.end());
  };
  while (!work.empty()) {
    WorkItem item = work.back();
    work.pop_back();
    TSQuery *q = item.tsset->query;
    if (!q)
      continue;
    TSQueryCursor *cursor = ts_query_cursor_new();
    ts_query_cursor_exec(cursor, q, ts_tree_root_node(item.tsset->tree));
    std::unordered_map<std::string, PendingRanges> pending_injections;
    TSQueryMatch match;
    while (ts_query_cursor_next_match(cursor, &match)) {
      auto subject_fn = [&](const TSNode *node) -> std::string {
        uint32_t start = ts_node_start_byte(*node);
        uint32_t end = ts_node_end_byte(*node);
        char *text = read(editor->root, start, end - start);
        std::string final = std::string(text, end - start);
        free(text);
        return final;
      };
      if (!ts_predicate(q, match, subject_fn))
        continue;
      for (uint32_t i = 0; i < match.capture_count; i++) {
        TSQueryCapture cap = match.captures[i];
        uint32_t start = ts_node_start_byte(cap.node);
        uint32_t end = ts_node_end_byte(cap.node);
        if (Highlight *hl = safe_get(item.tsset->query_map, cap.index))
          new_spans.push_back({start, end, hl});
        if (!injections_enabled)
          continue;
        if (Language *inj_lang =
                safe_get(item.tsset->injection_map, cap.index)) {
          auto &pending = pending_injections[inj_lang->name];
          TSSet &tsset =
              editor->ts.injections.try_emplace(inj_lang->name).first->second;
          if (!tsset.parser) {
            tsset.lang = inj_lang->name;
            tsset.parser = ts_parser_new();
            ts_parser_set_language(tsset.parser, inj_lang->fn());
            tsset.language = inj_lang->fn();
            tsset.query_file =
                get_exe_dir() + "/../grammar/" + inj_lang->name + ".scm";
            tsset.query = load_query(tsset.query_file.c_str(), &tsset);
          }
          pending.tsset = &tsset;
          pending.ranges.push_back(TSRange{
              ts_node_start_point(cap.node),
              ts_node_end_point(cap.node),
              start,
              end,
          });
        }
      }
    }
    ts_query_cursor_delete(cursor);
    if (injections_enabled && item.depth < kMaxInjectionDepth) {
      for (auto &[lang_name, pending] : pending_injections) {
        TSSet *tsset = pending.tsset;
        if (!tsset || pending.ranges.empty() || !tsset->parser || !tsset->query)
          continue;
        tsset->ranges = std::move(pending.ranges);
        remove_overlapping_spans(tsset->ranges);
        ts_parser_set_included_ranges(tsset->parser, tsset->ranges.data(),
                                      tsset->ranges.size());
        lock.lock();
        TSTree *tree = ts_parser_parse(tsset->parser, tsset->tree, tsinput);
        if (!tree)
          continue;
        if (tsset->tree)
          ts_tree_delete(tsset->tree);
        tsset->tree = tree;
        lock.unlock();
        work.push_back({reinterpret_cast<TSSetBase *>(tsset), tsset->tree,
                        item.depth + 1});
      }
    }
  }
  std::pair<uint32_t, int64_t> span_edit;
  while (editor->spans.edits.pop(span_edit))
    apply_edit(new_spans, span_edit.first, span_edit.second);
  std::sort(new_spans.begin(), new_spans.end());
  std::unique_lock span_mtx(editor->spans.mtx);
  editor->spans.mid_parse = false;
  editor->spans.spans.swap(new_spans);
}

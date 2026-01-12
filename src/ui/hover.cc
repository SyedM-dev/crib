#include "ui/hover.h"
#include "ts/ts.h"

void HoverBox::clear() {
  text = "";
  scroll_ = 0;
  is_markup = false;
  size = {0, 0};
  cells.clear();
}

void HoverBox::scroll(int32_t number) {
  if (text.empty() || number == 0)
    return;
  uint32_t line_count = 0;
  for (uint32_t i = 0; i < text.length(); i++)
    if (text[i] == '\n')
      line_count++;
  scroll_ = MAX((int32_t)scroll_ + number, 0);
  if (scroll_ > line_count)
    scroll_ = line_count;
  render_first(true);
}

void HoverBox::render_first(bool scroll) {
  if (!scroll) {
    std::vector<Span> base_spans;
    std::vector<Span> injected_spans;
    TSSetBase ts = TSSetBase{};
    if (is_markup) {
      highlights.clear();
      highlights.reserve(1024);
      base_spans.reserve(1024);
      injected_spans.reserve(1024);
      hover_spans.clear();
      hover_spans.reserve(1024);
      std::string query_path = get_exe_dir() + "/../grammar/hover.scm";
      ts.language = LANG(markdown)();
      ts.query = load_query(query_path.c_str(), &ts);
      ts.parser = ts_parser_new();
      ts_parser_set_language(ts.parser, ts.language);
      ts.tree = ts_parser_parse_string(ts.parser, nullptr, text.c_str(),
                                       text.length());
      TSQueryCursor *cursor = ts_query_cursor_new();
      ts_query_cursor_exec(cursor, ts.query, ts_tree_root_node(ts.tree));
      TSQueryMatch match;
      auto subject_fn = [&](const TSNode *node, uint32_t *len,
                            bool *allocated) -> char * {
        uint32_t start = ts_node_start_byte(*node);
        uint32_t end = ts_node_end_byte(*node);
        *len = end - start;
        *allocated = false;
        return text.data() + start;
      };
      while (ts_query_cursor_next_match(cursor, &match)) {
        if (!ts_predicate(ts.query, match, subject_fn))
          continue;
        for (uint32_t i = 0; i < match.capture_count; i++) {
          TSQueryCapture cap = match.captures[i];
          uint32_t start = ts_node_start_byte(cap.node);
          uint32_t end = ts_node_end_byte(cap.node);
          if (Language *inj_lang = safe_get(ts.injection_map, cap.index)) {
            TSSetBase inj_ts = TSSetBase{};
            inj_ts.language = inj_lang->fn();
            inj_ts.query_file =
                get_exe_dir() + "/../grammar/" + inj_lang->name + ".scm";
            inj_ts.query = load_query(inj_ts.query_file.c_str(), &inj_ts);
            inj_ts.parser = ts_parser_new();
            ts_parser_set_language(inj_ts.parser, inj_ts.language);
            TSPoint start_p = ts_node_start_point(cap.node);
            TSPoint end_p = ts_node_end_point(cap.node);
            std::vector<TSRange> ranges = {{start_p, end_p, start, end}};
            ts_parser_set_included_ranges(inj_ts.parser, ranges.data(), 1);
            inj_ts.tree = ts_parser_parse_string(inj_ts.parser, nullptr,
                                                 text.c_str(), text.length());
            TSQueryCursor *inj_cursor = ts_query_cursor_new();
            ts_query_cursor_exec(inj_cursor, inj_ts.query,
                                 ts_tree_root_node(inj_ts.tree));
            TSQueryMatch inj_match;
            while (ts_query_cursor_next_match(inj_cursor, &inj_match)) {
              if (!ts_predicate(inj_ts.query, inj_match, subject_fn))
                continue;
              for (uint32_t i = 0; i < inj_match.capture_count; i++) {
                TSQueryCapture inj_cap = inj_match.captures[i];
                uint32_t start = ts_node_start_byte(inj_cap.node);
                uint32_t end = ts_node_end_byte(inj_cap.node);
                if (Highlight *hl = safe_get(inj_ts.query_map, inj_cap.index)) {
                  if (highlights.size() >= 1000)
                    continue;
                  highlights.push_back(*hl);
                  Highlight *hl_f = &highlights.back();
                  injected_spans.push_back({start, end, hl_f});
                }
              }
            }
            ts_query_cursor_delete(inj_cursor);
            ts_tree_delete(inj_ts.tree);
            ts_parser_delete(inj_ts.parser);
            ts_query_delete(inj_ts.query);
            continue;
          }
          if (Highlight *hl = safe_get(ts.query_map, cap.index)) {
            if (highlights.size() >= 1000)
              continue;
            highlights.push_back(*hl);
            Highlight *hl_f = &highlights.back();
            base_spans.push_back({start, end, hl_f});
          }
        }
      }
      ts_query_cursor_delete(cursor);
      ts_query_delete(ts.query);
      ts_tree_delete(ts.tree);
      ts_parser_delete(ts.parser);
    }
    for (const auto &inj : injected_spans) {
      base_spans.erase(std::remove_if(base_spans.begin(), base_spans.end(),
                                      [&](const Span &base) {
                                        return !(base.end <= inj.start ||
                                                 base.start >= inj.end);
                                      }),
                       base_spans.end());
    }
    hover_spans.insert(hover_spans.end(), base_spans.begin(), base_spans.end());
    hover_spans.insert(hover_spans.end(), injected_spans.begin(),
                       injected_spans.end());
    std::sort(hover_spans.begin(), hover_spans.end());
  }
  uint32_t longest_line = 0;
  uint32_t current_width = 0;
  for (size_t j = 0; j < text.length(); j++) {
    if (text[j] == '\n') {
      longest_line = std::max(longest_line, current_width);
      current_width = 0;
    } else {
      current_width += 1;
    }
  }
  // HACK: the 1 is added so the longest line doesnt wrap which should be fixed
  //       in the loop instead as it was never meant to wrap in the first place
  longest_line = MAX(longest_line, current_width) + 1;
  uint32_t content_width = MIN(longest_line, 130u);
  size.col = content_width + 2;
  size_t i = 0;
  size_t lines_skipped = 0;
  while (i < text.length() && lines_skipped < scroll_) {
    if (text[i] == '\n')
      lines_skipped++;
    i++;
  }
  Spans spans{};
  spans.spans = hover_spans;
  uint32_t border_fg = 0x82AAFF;
  uint32_t base_bg = 0;
  SpanCursor span_cursor(spans);
  span_cursor.sync(i);
  cells.assign(size.col * 26, ScreenCell{" ", 0, 0, 0, 0, 0});
  auto set = [&](uint32_t r, uint32_t c, const char *text, uint32_t fg,
                 uint32_t bg, uint8_t flags) {
    cells[r * size.col + c] = {std::string(text), 0, fg, bg, flags, 0};
  };
  uint32_t r = 0;
  while (i < text.length() && r < 24) {
    uint32_t c = 0;
    while (c < content_width && i < text.length()) {
      if (text[i] == '\n') {
        while (i < text.length() && text[i] == '\n')
          i++;
        break;
      }
      uint32_t cluster_len = grapheme_next_character_break_utf8(
          text.c_str() + i, text.length() - i);
      std::string cluster = text.substr(i, cluster_len);
      int width = display_width(cluster.c_str(), cluster_len);
      if (c + width > content_width)
        break;
      Highlight *hl = span_cursor.get_highlight(i);
      uint32_t fg = hl ? hl->fg : 0xFFFFFF;
      uint32_t bg = hl ? hl->bg : 0;
      uint32_t flags = hl ? hl->flags : 0;
      set(r + 1, c + 1, cluster.c_str(), fg, bg | base_bg, flags);
      c += width;
      i += cluster_len;
      for (int w = 1; w < width; w++)
        set(r + 1, c - w + 1, "\x1b", 0xFFFFFF, base_bg, 0);
    }
    r++;
  }
  if (!scroll)
    size.row = r + 2;
  set(0, 0, "┌", border_fg, base_bg, 0);
  for (uint32_t i = 1; i < size.col - 1; i++)
    set(0, i, "─", border_fg, base_bg, 0);
  set(0, size.col - 1, "┐", border_fg, base_bg, 0);
  for (uint32_t r = 1; r < size.row - 1; r++) {
    set(r, 0, "│", border_fg, base_bg, 0);
    set(r, size.col - 1, "│", border_fg, base_bg, 0);
  }
  set(size.row - 1, 0, "└", border_fg, base_bg, 0);
  for (uint32_t i = 1; i < size.col - 1; i++)
    set(size.row - 1, i, "─", border_fg, base_bg, 0);
  set(size.row - 1, size.col - 1, "┘", border_fg, base_bg, 0);
  cells.resize(size.col * size.row);
}

void HoverBox::render(Coord pos) {
  int32_t start_row = (int32_t)pos.row - (int32_t)size.row;
  if (start_row < 0)
    start_row = pos.row + 1;
  int32_t start_col = pos.col;
  if (start_col + size.col > cols) {
    start_col = cols - size.col;
    if (start_col < 0)
      start_col = 0;
  }
  for (uint32_t r = 0; r < size.row; r++)
    for (uint32_t c = 0; c < size.col; c++)
      update(start_row + r, start_col + c, cells[r * size.col + c].utf8,
             cells[r * size.col + c].fg, cells[r * size.col + c].bg,
             cells[r * size.col + c].flags);
}

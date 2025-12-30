#include "editor/editor.h"
#include "ts/ts.h"

static Highlight HL_UNDERLINE = {0, 0, CF_UNDERLINE, UINT8_MAX - 1};

void hover_diagnostic(Editor *editor) {
  std::shared_lock lock(editor->v_mtx);
  static uint32_t last_line = UINT32_MAX;
  if (last_line == editor->cursor.row && !editor->warnings_dirty)
    return;
  VWarn dummy;
  dummy.line = editor->cursor.row;
  editor->warnings_dirty = false;
  last_line = editor->cursor.row;
  auto first =
      std::lower_bound(editor->warnings.begin(), editor->warnings.end(), dummy);
  auto last =
      std::upper_bound(editor->warnings.begin(), editor->warnings.end(), dummy);
  std::vector<VWarn> warnings_at_line(first, last);
  if (warnings_at_line.size() == 0) {
    editor->diagnostics_active = false;
    return;
  }
  editor->diagnostics.clear();
  editor->diagnostics.warnings.swap(warnings_at_line);
  editor->diagnostics.render_first();
  editor->diagnostics_active = true;
}

void editor_worker(Editor *editor) {
  if (!editor || !editor->root)
    return;
  if (editor->root->char_count > (1024 * 200))
    return;
  if (editor->ts.query_file != "" && !editor->ts.query)
    editor->ts.query = load_query(editor->ts.query_file.c_str(), &editor->ts);
  if (editor->ts.parser && editor->ts.query)
    ts_collect_spans(editor);
  uint32_t prev_col, next_col;
  word_boundaries_exclusive(editor, editor->cursor, &prev_col, &next_col);
  std::unique_lock lock(editor->def_spans.mtx);
  editor->def_spans.spans.clear();
  if (next_col - prev_col > 0 && next_col - prev_col < 256 - 4) {
    std::shared_lock lockk(editor->knot_mtx);
    uint32_t offset = line_to_byte(editor->root, editor->cursor.row, nullptr);
    char *word = read(editor->root, offset + prev_col, next_col - prev_col);
    lockk.unlock();
    if (word) {
      char buf[256];
      snprintf(buf, sizeof(buf), "\\b%s\\b", word);
      std::vector<std::pair<size_t, size_t>> results =
          search_rope(editor->root, buf);
      for (const auto &match : results) {
        Span s;
        s.start = match.first;
        s.end = match.first + match.second;
        s.hl = &HL_UNDERLINE;
        editor->def_spans.spans.push_back(s);
      }
      free(word);
    }
  }
  uint8_t top = 0;
  static Highlight *hl_s = (Highlight *)calloc(200, sizeof(Highlight));
  if (!hl_s)
    exit(ENOMEM);
  std::shared_lock lockk(editor->knot_mtx);
  std::vector<std::pair<size_t, size_t>> results =
      search_rope(editor->root, "(0x|#)[0-9a-fA-F]{6}");
  for (int i = 0; i < results.size() && top < 200; i++) {
    Span s;
    s.start = results[i].first;
    s.end = results[i].first + results[i].second;
    char *buf = read(editor->root, s.start, s.end - s.start);
    int x = buf[0] == '#' ? 1 : 2;
    uint32_t bg = HEX(buf + x);
    free(buf);
    uint8_t r = bg >> 16, g = (bg >> 8) & 0xFF, b = bg & 0xFF;
    double luminance = 0.299 * r + 0.587 * g + 0.114 * b;
    uint32_t fg = (luminance > 128) ? 0x010101 : 0xFEFEFE;
    hl_s[top] = {fg, bg, CF_BOLD, UINT8_MAX};
    s.hl = &hl_s[top];
    editor->def_spans.spans.push_back(s);
    top++;
  }
  std::sort(editor->def_spans.spans.begin(), editor->def_spans.spans.end());
  lock.unlock();
  lockk.unlock();
  hover_diagnostic(editor);
}

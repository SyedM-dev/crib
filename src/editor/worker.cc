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
  if (editor->root->char_count > (1024 * 128))
    return;
  if (editor->ts.query_file != "" && !editor->ts.query)
    editor->ts.query = load_query(editor->ts.query_file.c_str(), &editor->ts);
  if (editor->ts.parser && editor->ts.query)
    ts_collect_spans(editor);
  if (editor->root->char_count > (1024 * 32))
    return;
  uint32_t prev_col, next_col;
  word_boundaries_exclusive(editor, editor->cursor, &prev_col, &next_col);
  std::unique_lock lock(editor->word_spans.mtx);
  editor->word_spans.spans.clear();
  lock.unlock();
  if (next_col - prev_col > 0 && next_col - prev_col < 256 - 4) {
    std::shared_lock lockk(editor->knot_mtx);
    uint32_t offset = line_to_byte(editor->root, editor->cursor.row, nullptr);
    char *word = read(editor->root, offset + prev_col, next_col - prev_col);
    lockk.unlock();
    if (word) {
      char buf[256];
      snprintf(buf, sizeof(buf), "\\b%s\\b", word);
      std::shared_lock lockk(editor->knot_mtx);
      std::vector<std::pair<size_t, size_t>> results =
          search_rope_dfa(editor->root, buf);
      lockk.unlock();
      std::unique_lock lock2(editor->word_spans.mtx);
      editor->word_spans.spans.reserve(results.size());
      for (const auto &match : results) {
        Span s;
        s.start = match.first;
        s.end = match.first + match.second;
        s.hl = &HL_UNDERLINE;
        editor->word_spans.spans.push_back(s);
      }
      free(word);
      lock2.unlock();
    }
  }
  static uint16_t limit = 150;
  static Highlight *hl_s = (Highlight *)calloc(limit, sizeof(Highlight));
  if (!hl_s)
    exit(ENOMEM);
  std::shared_lock lockk(editor->knot_mtx);
  std::vector<Match> results =
      search_rope(editor->root, "(?:0x|#)[0-9a-fA-F]{6,8}\\b");
  if (results.size() > limit) {
    limit = results.size() + 50;
    free(hl_s);
    hl_s = (Highlight *)calloc(limit, sizeof(Highlight));
    if (!hl_s)
      exit(ENOMEM);
  }
  lockk.unlock();
  std::unique_lock lock2(editor->hex_color_spans.mtx);
  editor->hex_color_spans.spans.clear();
  editor->hex_color_spans.spans.reserve(results.size());
  for (size_t i = 0; i < results.size(); ++i) {
    Span s;
    s.start = results[i].start;
    s.end = results[i].end;
    int x = results[i].text[0] == '#' ? 1 : 2;
    uint32_t bg = HEX(results[i].text.substr(x, 6));
    uint8_t r = bg >> 16;
    uint8_t g = (bg >> 8) & 0xFF;
    uint8_t b = bg & 0xFF;
    double luminance = 0.299 * r + 0.587 * g + 0.114 * b;
    uint32_t fg = (luminance > 128) ? 0x010101 : 0xFEFEFE;
    hl_s[i] = {fg, bg, CF_BOLD, UINT8_MAX};
    s.hl = &hl_s[i];
    editor->hex_color_spans.spans.push_back(s);
  }
  lock2.unlock();
  hover_diagnostic(editor);
}

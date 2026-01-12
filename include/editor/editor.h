#ifndef EDITOR_H
#define EDITOR_H

#include "editor/completions.h"
#include "editor/indents.h"
#include "editor/spans.h"
#include "io/knot.h"
#include "io/sysio.h"
#include "ts/decl.h"
#include "ui/completionbox.h"
#include "ui/diagnostics.h"
#include "ui/hover.h"
#include "utils/utils.h"

#define CHAR 0
#define WORD 1
#define LINE 2

#define EXTRA_META 4
#define INDENT_WIDTH 2

// autocomplete lua// Bracket closing / tab on enter

struct Editor {
  std::string filename;
  std::string uri;
  Knot *root;
  std::shared_mutex knot_mtx;
  Coord cursor;
  uint32_t cursor_preffered;
  Coord selection;
  bool selection_active;
  int selection_type;
  Coord position;
  Coord size;
  Coord scroll;
  Language lang;
  TSSetMain ts;
  Queue<TSInputEdit> edit_queue;
  std::vector<Fold> folds;
  Spans spans;
  Spans word_spans;
  Spans hex_color_spans;
  uint32_t hooks[94];
  bool jumper_set;
  std::shared_mutex v_mtx;
  std::vector<VWarn> warnings;
  bool warnings_dirty;
  VAI ai;
  std::shared_mutex lsp_mtx;
  std::shared_ptr<struct LSPInstance> lsp;
  bool hover_active;
  HoverBox hover;
  bool diagnostics_active;
  DiagnosticBox diagnostics;
  std::atomic<int> lsp_version = 1;
  CompletionSession completion;
  IndentationEngine indents;
};

Editor *new_editor(const char *filename_arg, Coord position, Coord size);
void save_file(Editor *editor);
void free_editor(Editor *editor);
void render_editor(Editor *editor);
void fold(Editor *editor, uint32_t start_line, uint32_t end_line);
void cursor_up(Editor *editor, uint32_t number);
void cursor_down(Editor *editor, uint32_t number);
Coord move_left(Editor *editor, Coord cursor, uint32_t number);
Coord move_right(Editor *editor, Coord cursor, uint32_t number);
Coord move_left_pure(Editor *editor, Coord cursor, uint32_t number);
Coord move_right_pure(Editor *editor, Coord cursor, uint32_t number);
void cursor_left(Editor *editor, uint32_t number);
void cursor_right(Editor *editor, uint32_t number);
void scroll_up(Editor *editor, int32_t number);
void scroll_down(Editor *editor, uint32_t number);
void ensure_cursor(Editor *editor);
void ensure_scroll(Editor *editor);
void handle_editor_event(Editor *editor, KeyEvent event);
void edit_erase(Editor *editor, Coord pos, int64_t len);
void edit_insert(Editor *editor, Coord pos, char *data, uint32_t len);
void edit_replace(Editor *editor, Coord start, Coord end, const char *text,
                  uint32_t len);
Coord editor_hit_test(Editor *editor, uint32_t x, uint32_t y);
char *get_selection(Editor *editor, uint32_t *out_len, Coord *out_start);
void editor_worker(Editor *editor);
void move_line_down(Editor *editor);
void move_line_up(Editor *editor);
void word_boundaries(Editor *editor, Coord coord, uint32_t *prev_col,
                     uint32_t *next_col, uint32_t *prev_clusters,
                     uint32_t *next_clusters);
void word_boundaries_exclusive(Editor *editor, Coord coord, uint32_t *prev_col,
                               uint32_t *next_col);
std::vector<Fold>::iterator find_fold_iter(Editor *editor, uint32_t line);
bool add_fold(Editor *editor, uint32_t start, uint32_t end);
bool remove_fold(Editor *editor, uint32_t line);
void editor_lsp_handle(Editor *editor, json msg);
void apply_lsp_edits(Editor *editor, std::vector<TextEdit> edits, bool move);
void completion_resolve_doc(Editor *editor);
void complete_accept(Editor *editor);
void complete_next(Editor *editor);
void complete_prev(Editor *editor);
void complete_select(Editor *editor, uint8_t index);
void handle_completion(Editor *editor, KeyEvent event);

inline void apply_hook_insertion(Editor *editor, uint32_t line, uint32_t rows) {
  for (auto &hook : editor->hooks)
    if (hook > line)
      hook += rows;
}

inline void apply_hook_deletion(Editor *editor, uint32_t removal_start,
                                uint32_t removal_end) {
  for (auto &hook : editor->hooks)
    if (hook > removal_start)
      hook -= removal_end - removal_start + 1;
}

inline static void utf8_normalize_edit(Editor *editor, TextEdit *edit) {
  std::shared_lock lock(editor->knot_mtx);
  if (edit->start.row > editor->root->line_count) {
    edit->start.row = editor->root->line_count;
    edit->start.col = UINT32_MAX;
  }
  if (edit->end.row > editor->root->line_count) {
    edit->end.row = editor->root->line_count;
    edit->end.col = UINT32_MAX;
  }
  LineIterator *it = begin_l_iter(editor->root, edit->start.row);
  if (!it)
    return;
  uint32_t len;
  char *line = next_line(it, &len);
  if (!line) {
    free(it->buffer);
    free(it);
    return;
  }
  if (edit->start.col < len)
    edit->start.col = utf16_offset_to_utf8(line, edit->start.col);
  else
    edit->start.col = len;
  if (edit->end.row == edit->start.row) {
    if (edit->end.col < len)
      edit->end.col = utf16_offset_to_utf8(line, edit->end.col);
    else
      edit->end.col = len;
    free(it->buffer);
    free(it);
    return;
  }
  free(it->buffer);
  free(it);
  it = begin_l_iter(editor->root, edit->end.row);
  if (!it)
    return;
  line = next_line(it, &len);
  if (!line) {
    free(it->buffer);
    free(it);
    return;
  }
  if (edit->end.col < len)
    edit->end.col = utf16_offset_to_utf8(line, edit->end.col);
  else
    edit->end.col = len;
  free(it->buffer);
  free(it);
}

#endif

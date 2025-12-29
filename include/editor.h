#ifndef EDITOR_H
#define EDITOR_H

#include "./hover.h"
#include "./knot.h"
#include "./pch.h"
#include "./spans.h"
#include "./ts_def.h"
#include "./ui.h"
#include "./utils.h"

#define CHAR 0
#define WORD 1
#define LINE 2

#define EXTRA_META 4
#define INDENT_WIDTH 2

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
  TSSetMain ts;
  Queue<TSInputEdit> edit_queue;
  std::vector<Fold> folds;
  Spans spans;
  // TODO: Split into 2 groups to have their own mutex's . one for word hl and
  //       one for hex colors
  Spans def_spans;
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
  int lsp_version = 1;
};

inline const Fold *fold_for_line(const std::vector<Fold> &folds,
                                 uint32_t line) {
  auto it = std::lower_bound(
      folds.begin(), folds.end(), line,
      [](const Fold &fold, uint32_t value) { return fold.start < value; });
  if (it != folds.end() && it->start == line)
    return &(*it);
  if (it != folds.begin()) {
    --it;
    if (it->contains(line))
      return &(*it);
  }
  return nullptr;
}

inline Fold *fold_for_line(std::vector<Fold> &folds, uint32_t line) {
  const auto *fold =
      fold_for_line(static_cast<const std::vector<Fold> &>(folds), line);
  return const_cast<Fold *>(fold);
}

inline bool line_is_fold_start(const std::vector<Fold> &folds, uint32_t line) {
  const Fold *fold = fold_for_line(folds, line);
  return fold && fold->start == line;
}

inline bool line_is_folded(const std::vector<Fold> &folds, uint32_t line) {
  return fold_for_line(folds, line) != nullptr;
}

inline uint32_t next_unfolded_row(const Editor *editor, uint32_t row) {
  uint32_t limit = editor && editor->root ? editor->root->line_count : 0;
  while (row < limit) {
    const Fold *fold = fold_for_line(editor->folds, row);
    if (!fold)
      return row;
    row = fold->end + 1;
  }
  return limit;
}

inline uint32_t prev_unfolded_row(const Editor *editor, uint32_t row) {
  while (row > 0) {
    const Fold *fold = fold_for_line(editor->folds, row);
    if (!fold)
      return row;
    if (fold->start == 0)
      return 0;
    row = fold->start - 1;
  }
  return 0;
}

void apply_edit(std::vector<Span> &spans, uint32_t x, int64_t y);
void apply_hook_insertion(Editor *editor, uint32_t line, uint32_t rows);
void apply_hook_deletion(Editor *editor, uint32_t removal_start,
                         uint32_t removal_end);
Editor *new_editor(const char *filename_arg, Coord position, Coord size);
void save_file(Editor *editor);
void hover_diagnostic(Editor *editor);
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
void indent_line(Editor *editor, uint32_t row);
void dedent_line(Editor *editor, uint32_t row);
void ensure_scroll(Editor *editor);
void handle_editor_event(Editor *editor, KeyEvent event);
void edit_erase(Editor *editor, Coord pos, int64_t len);
void edit_insert(Editor *editor, Coord pos, char *data, uint32_t len);
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
void apply_line_insertion(Editor *editor, uint32_t line, uint32_t rows);
void apply_line_deletion(Editor *editor, uint32_t removal_start,
                         uint32_t removal_end);
uint32_t leading_indent(const char *line, uint32_t len);
uint32_t get_indent(Editor *editor, Coord cursor);
bool closing_after_cursor(const char *line, uint32_t len, uint32_t col);
void editor_lsp_handle(Editor *editor, json msg);

#endif

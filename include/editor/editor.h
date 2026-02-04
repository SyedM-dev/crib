#ifndef EDITOR_H
#define EDITOR_H

#include "editor/indents.h"
#include "io/knot.h"
#include "io/sysio.h"
#include "syntax/extras.h"
#include "syntax/parser.h"
#include "ui/diagnostics.h"
#include "utils/utils.h"
#include "windows/decl.h"

#define CHAR 0
#define WORD 1
#define LINE 2

#define EXTRA_META 2
#define INDENT_WIDTH 2

struct Editor : Window {
  std::string filename = "";
  std::string uri = "";
  Knot *root = nullptr;
  Coord cursor = {0, 0};
  uint32_t cursor_preffered = 0;
  Coord selection = {0, 0};
  bool selection_active = false;
  bool unix_eol = true;
  int selection_type = 0;
  Coord size = {0, 0};
  Coord scroll = {0, 0};
  Language lang = {};
  uint32_t hooks[94] = {0};
  bool jumper_set = false;
  std::vector<VWarn> warnings = {};
  bool warnings_dirty = false;
  VAI ai = {};
  std::shared_mutex lsp_mtx;
  std::atomic<struct LSPInstance *> lsp = nullptr;
  bool hover_active = false;
  bool diagnostics_active = false;
  std::atomic<int> lsp_version = 1;
  IndentationEngine indents = {};
  Parser *parser = nullptr;
  ExtraHighlighter extra_hl = {};
  bool is_css_color = false;

  Editor(const char *filename_arg, uint8_t eol);
  ~Editor();

  void render(std::vector<ScreenCell> &buffer, Coord size, Coord pos) override;
  void handle_event(KeyEvent event) override;
  void handle_click(KeyEvent event, Coord size) override;
  void handle_command(std::string &command) override;
  void work() override;
  std::array<std::string, 5> bar_info() override { return {}; };

  void save();
  void cursor_up(uint32_t number);
  void cursor_down(uint32_t number);
  void cursor_left(uint32_t number);
  void cursor_right(uint32_t number);
  void move_line_down();
  void move_line_up();

  void scroll_up(uint32_t number);
  void scroll_down(uint32_t number);
  void ensure_cursor();
  void ensure_scroll();

  void edit_erase(Coord pos, int64_t len);
  void edit_insert(Coord pos, char *data, uint32_t len);
  void edit_replace(Coord start, Coord end, const char *text, uint32_t len);

  Coord click_coord(uint32_t x, uint32_t y);

  char *get_selection(uint32_t *out_len, Coord *out_start);
  void selection_bounds(Coord *out_start, Coord *out_end);

  void insert_str(char *c, uint32_t len);
  void insert_char(char c);
  void normal_mode();
  void backspace_edit();
  void delete_prev_word();
  void delete_next_word();
  void clear_hooks_at_line(uint32_t line);
  void cursor_prev_word();
  void cursor_next_word();
  void select_all();
  void fetch_lsp_hover();
  void indent_current_line();
  void dedent_current_line();
  void indent_selection();
  void dedent_selection();
  void paste();
  void copy();
  void cut();

  void lsp_handle(json msg);
  void apply_lsp_edits(std::vector<TextEdit> edits, bool move);

  Coord move_left(Coord cursor, uint32_t number);
  Coord move_right(Coord cursor, uint32_t number);

  void word_boundaries(Coord coord, uint32_t *prev_col, uint32_t *next_col,
                       uint32_t *prev_clusters, uint32_t *next_clusters);
  void word_boundaries_exclusive(Coord coord, uint32_t *prev_col,
                                 uint32_t *next_col);

  void utf8_normalize_edit(TextEdit *edit) {
    if (edit->start.row > this->root->line_count) {
      edit->start.row = this->root->line_count;
      edit->start.col = UINT32_MAX;
    }
    if (edit->end.row > this->root->line_count) {
      edit->end.row = this->root->line_count;
      edit->end.col = UINT32_MAX;
    }
    LineIterator *it = begin_l_iter(this->root, edit->start.row);
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
      edit->start.col = utf16_offset_to_utf8(line, len, edit->start.col);
    else
      edit->start.col = len;
    if (edit->end.row == edit->start.row) {
      if (edit->end.col < len)
        edit->end.col = utf16_offset_to_utf8(line, len, edit->end.col);
      else
        edit->end.col = len;
      free(it->buffer);
      free(it);
      return;
    }
    free(it->buffer);
    free(it);
    it = begin_l_iter(this->root, edit->end.row);
    if (!it)
      return;
    line = next_line(it, &len);
    if (!line) {
      free(it->buffer);
      free(it);
      return;
    }
    if (edit->end.col < len)
      edit->end.col = utf16_offset_to_utf8(line, len, edit->end.col);
    else
      edit->end.col = len;
    free(it->buffer);
    free(it);
  }

  inline void apply_hook_insertion(uint32_t line, uint32_t rows) {
    for (auto &hook : this->hooks)
      if (hook > line)
        hook += rows;
  }

  inline void apply_hook_deletion(uint32_t removal_start,
                                  uint32_t removal_end) {
    for (auto &hook : this->hooks)
      if (hook > removal_start)
        hook -= removal_end - removal_start + 1;
  }
};

#endif

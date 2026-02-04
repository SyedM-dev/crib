#include "editor/helpers.h"
#include "editor/editor.h"
#include "extentions/hover.h"
#include "io/sysio.h"
#include "lsp/lsp.h"
#include "main.h"
#include "utils/utils.h"

void Editor::cut() {
  if (ABS((int64_t)this->cursor.row - (int64_t)this->selection.row) > 1500) {
    ui::bar.log("Selection too large!");
    return;
  }
  if (mode != SELECT)
    return;
  Coord start;
  uint32_t len;
  char *text = this->get_selection(&len, &start);
  ruby_copy(text, len);
  len = count_clusters(text, len, 0, len);
  this->edit_erase(start, len);
  free(text);
  this->selection_active = false;
}

void Editor::copy() {
  if (ABS((int64_t)this->cursor.row - (int64_t)this->selection.row) > 1500) {
    ui::bar.log("Selection too large!");
    return;
  }
  if (mode != SELECT)
    return;
  uint32_t len;
  char *text = this->get_selection(&len, nullptr);
  ruby_copy(text, len);
  free(text);
  this->selection_active = false;
}

void Editor::paste() {
  if (mode == NORMAL) {
    std::string text = ruby_paste();
    if (text.empty())
      return;
    this->insert_str((char *)text.c_str(), text.length());
  } else if (mode == SELECT) {
    std::string text = ruby_paste();
    if (!text.empty()) {
      Coord start, end;
      this->selection_bounds(&start, &end);
      uint32_t start_byte =
          line_to_byte(this->root, start.row, nullptr) + start.col;
      uint32_t end_byte = line_to_byte(this->root, end.row, nullptr) + end.col;
      this->edit_erase(start, end_byte - start_byte);
      this->edit_insert(this->cursor, (char *)text.c_str(), text.length());
    }
    this->selection_active = false;
  }
}

void Editor::insert_str(char *c, uint32_t len) {
  if (c) {
    this->edit_insert(this->cursor, c, len);
    uint32_t grapheme_len = count_clusters(c, len, 0, len);
    this->cursor_right(grapheme_len);
  }
}

void Editor::indent_current_line() {
  Coord start = this->cursor;
  uint32_t delta = this->indents.indent_line(this->cursor.row);
  this->cursor.col = start.col + delta;
  this->cursor.row = start.row;
}

void Editor::dedent_current_line() {
  Coord start = this->cursor;
  uint32_t delta = this->indents.dedent_line(this->cursor.row);
  this->cursor.col = MAX((int64_t)start.col - delta, 0);
  this->cursor.row = start.row;
}

static void move_coord_by_delta(Coord &c, uint32_t row, int64_t delta) {
  if (c.row == row) {
    int64_t new_col = (int64_t)c.col + delta;
    c.col = (uint32_t)MAX(new_col, 0);
  }
}

void Editor::indent_selection() {
  uint32_t top = MIN(this->cursor.row, this->selection.row);
  uint32_t bot = MAX(this->cursor.row, this->selection.row);
  if (bot - top > 1500) {
    ui::bar.log("Can't indent more than 1500 lines at once!");
    return;
  }
  if (bot - top >= 2)
    this->indents.indent_block(top + 1, bot - 1);
  uint32_t delta_top = this->indents.indent_line(top);
  uint32_t delta_bot =
      (bot == top) ? delta_top : this->indents.indent_line(bot);
  move_coord_by_delta(this->cursor, top, delta_top);
  move_coord_by_delta(this->selection, top, delta_top);
  if (bot != top) {
    move_coord_by_delta(this->cursor, bot, delta_bot);
    move_coord_by_delta(this->selection, bot, delta_bot);
  }
}

void Editor::dedent_selection() {
  uint32_t top = MIN(this->cursor.row, this->selection.row);
  uint32_t bot = MAX(this->cursor.row, this->selection.row);
  if (bot - top > 1500) {
    ui::bar.log("Can't dedent more than 1500 lines at once!");
    return;
  }
  if (bot - top >= 2)
    this->indents.dedent_block(top + 1, bot - 1);
  uint32_t delta_top = this->indents.dedent_line(top);
  uint32_t delta_bot =
      (bot == top) ? delta_top : this->indents.dedent_line(bot);
  move_coord_by_delta(this->cursor, top, -(int64_t)delta_top);
  move_coord_by_delta(this->selection, top, -(int64_t)delta_top);
  if (bot != top) {
    move_coord_by_delta(this->cursor, bot, -(int64_t)delta_bot);
    move_coord_by_delta(this->selection, bot, -(int64_t)delta_bot);
  }
}

void Editor::insert_char(char c) {
  uint32_t col = this->cursor.col;
  LineIterator *it = begin_l_iter(this->root, this->cursor.row);
  if (!it)
    return;
  uint32_t len;
  char *line = next_line(it, &len);
  if (!line) {
    free(it->buffer);
    free(it);
    return;
  }
  bool skip_insert = false;
  if (line && col < len) {
    char next = line[col];
    if ((c == '}' && next == '}') || (c == ')' && next == ')') ||
        (c == ']' && next == ']') || (c == '"' && next == '"') ||
        (c == '\'' && next == '\'')) {
      this->cursor_right(1);
      skip_insert = true;
    }
  }
  free(it->buffer);
  free(it);
  if (!skip_insert) {
    char closing = 0;
    switch (c) {
    case '{':
      closing = '}';
      break;
    case '(':
      closing = ')';
      break;
    case '[':
      closing = ']';
      break;
    case '"':
      closing = '"';
      break;
    case '\'':
      closing = '\'';
      break;
    }
    if (closing) {
      char pair[2] = {c, closing};
      this->edit_insert(this->cursor, pair, 2);
      this->cursor_right(1);
    } else {
      this->edit_insert(this->cursor, &c, 1);
      this->cursor_right(1);
    }
    auto lsp = this->lsp.load();
    if (lsp && lsp->allow_formatting_on_type) {
      for (char ch : lsp->format_chars) {
        if (ch == c) {
          LineIterator *it = begin_l_iter(this->root, this->cursor.row);
          if (!it)
            return;
          uint32_t len;
          char *line = next_line(it, &len);
          if (!line) {
            free(it->buffer);
            free(it);
            return;
          }
          uint32_t col = utf8_offset_to_utf16(line, len, this->cursor.col);
          free(it->buffer);
          free(it);
          int version = this->lsp_version;
          auto message = std::make_unique<LSPMessage>();
          message->message = {
              {"method", "textDocument/onTypeFormatting"},
              {"params",
               {{"textDocument", {{"uri", this->uri}}},
                {"position", {{"line", this->cursor.row}, {"character", col}}},
                {"ch", std::string(1, c)},
                {"options",
                 {{"tabSize", 2},
                  {"insertSpaces", true},
                  {"trimTrailingWhitespace", true},
                  {"trimFinalNewlines", true}}}}}};
          message->editor = this;
          message->callback = [version](const LSPMessage &message) {
            if (version != message.editor->lsp_version)
              return;
            auto &edits = message.message["result"];
            if (edits.is_array()) {
              std::vector<TextEdit> t_edits;
              t_edits.reserve(edits.size());
              for (auto &edit : edits) {
                TextEdit t_edit;
                t_edit.text = edit.value("newText", "");
                t_edit.start.row = edit["range"]["start"]["line"];
                t_edit.start.col = edit["range"]["start"]["character"];
                t_edit.end.row = edit["range"]["end"]["line"];
                t_edit.end.col = edit["range"]["end"]["character"];
                message.editor->utf8_normalize_edit(&t_edit);
                t_edits.push_back(t_edit);
              }
              message.editor->apply_lsp_edits(t_edits, false);
              message.editor->ensure_scroll();
            }
          };
          lsp->send(std::move(message));
          break;
        }
      }
    }
  }
}

void Editor::normal_mode() {
  Coord prev_pos = this->cursor;
  mode = NORMAL;
  this->cursor_left(1);
  if (prev_pos.row != this->cursor.row)
    this->cursor_right(1);
}

void Editor::backspace_edit() {
  Coord prev_pos = this->cursor;
  if (prev_pos.col > 0)
    prev_pos.col--;
  LineIterator *it = begin_l_iter(this->root, this->cursor.row);
  if (!it)
    return;
  uint32_t len;
  char *line = next_line(it, &len);
  if (!line) {
    free(it->buffer);
    free(it);
    return;
  }
  if (len > 0 && line[len - 1] == '\n')
    --len;
  char prev_char = (prev_pos.col < len) ? line[prev_pos.col] : 0;
  char next_char = (this->cursor.col < len) ? line[this->cursor.col] : 0;
  bool before_content = false;
  if (this->cursor.col > 0) {
    before_content = true;
    for (uint32_t i = 0; i < this->cursor.col; i++)
      if (line[i] != ' ' && line[i] != '\t') {
        before_content = false;
        break;
      }
  }
  free(it->buffer);
  free(it);
  if (before_content) {
    this->dedent_current_line();
    return;
  }
  bool is_pair = (prev_char == '{' && next_char == '}') ||
                 (prev_char == '(' && next_char == ')') ||
                 (prev_char == '[' && next_char == ']') ||
                 (prev_char == '"' && next_char == '"') ||
                 (prev_char == '\'' && next_char == '\'');
  if (is_pair) {
    this->edit_erase(this->cursor, 1);
    this->edit_erase(prev_pos, 1);
  } else {
    this->edit_erase(this->cursor, -1);
  }
}

void Editor::delete_prev_word() {
  uint32_t prev_col_byte, prev_col_cluster;
  this->word_boundaries(this->cursor, &prev_col_byte, nullptr,
                        &prev_col_cluster, nullptr);
  if (prev_col_byte == this->cursor.col)
    this->edit_erase(this->cursor, -1);
  else
    this->edit_erase(this->cursor, -(int64_t)prev_col_cluster);
}

void Editor::delete_next_word() {
  uint32_t next_col_byte, next_col_cluster;
  this->word_boundaries(this->cursor, nullptr, &next_col_byte, nullptr,
                        &next_col_cluster);
  if (next_col_byte == this->cursor.col)
    this->edit_erase(this->cursor, 1);
  else
    this->edit_erase(this->cursor, next_col_cluster);
}

void Editor::clear_hooks_at_line(uint32_t line) {
  for (uint8_t i = 0; i < 94; i++)
    if (this->hooks[i] == line + 1) {
      this->hooks[i] = 0;
      break;
    }
}

void Editor::cursor_prev_word() {
  uint32_t prev_col;
  word_boundaries(this->cursor, &prev_col, nullptr, nullptr, nullptr);
  this->cursor_preffered = UINT32_MAX;
  if (prev_col == this->cursor.col)
    cursor_left(1);
  else
    this->cursor = {this->cursor.row, prev_col};
}

void Editor::cursor_next_word() {
  uint32_t next_col;
  word_boundaries(this->cursor, nullptr, &next_col, nullptr, nullptr);
  this->cursor_preffered = UINT32_MAX;
  if (next_col == this->cursor.col)
    this->cursor_right(1);
  else
    this->cursor = {this->cursor.row, next_col};
}

void Editor::select_all() {
  if (this->root->line_count > 0) {
    this->cursor.row = this->root->line_count - 1;
    LineIterator *it = begin_l_iter(this->root, this->cursor.row);
    if (!it)
      return;
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line)
      return;
    if (line_len > 0 && line[line_len - 1] == '\n')
      line_len--;
    line_len = count_clusters(line, line_len, 0, line_len);
    free(it->buffer);
    free(it);
    this->cursor.col = line_len;
    this->cursor_preffered = UINT32_MAX;
    mode = SELECT;
    this->selection_active = true;
    this->selection = {0, 0};
    this->selection_type = LINE;
  }
}

void Editor::fetch_lsp_hover() {
  auto lsp = this->lsp.load();
  if (lsp && lsp->allow_hover) {
    LineIterator *it = begin_l_iter(this->root, this->cursor.row);
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line) {
      free(it->buffer);
      free(it);
      return;
    }
    uint32_t col = utf8_offset_to_utf16(line, line_len, this->cursor.col);
    free(it->buffer);
    free(it);
    auto message = std::make_unique<LSPMessage>();
    message->message = {
        {"method", "textDocument/hover"},
        {"params",
         {{"textDocument", {{"uri", this->uri}}},
          {"position", {{"line", this->cursor.row}, {"character", col}}}}}};
    message->editor = this;
    message->callback = [](const LSPMessage &message) {
      auto &hover = message.message;
      if (hover.contains("result") && !hover["result"].is_null()) {
        auto &contents = hover["result"]["contents"];
        std::string hover_text = "";
        bool is_markup = false;
        if (contents.is_object()) {
          hover_text += contents["value"].get<std::string>();
          is_markup = (contents["kind"].get<std::string>() == "markdown");
        } else if (contents.is_array()) {
          for (auto &block : contents) {
            if (block.is_string()) {
              hover_text += block.get<std::string>() + "\n";
            } else if (block.is_object() && block.contains("language") &&
                       block.contains("value")) {
              std::string lang = block["language"].get<std::string>();
              std::string val = block["value"].get<std::string>();
              is_markup = true;
              hover_text += "```" + lang + "\n" + val + "\n```\n";
            }
          }
        } else if (contents.is_string()) {
          hover_text += contents.get<std::string>();
        }
        if (!hover_text.empty()) {
          auto hover_box = static_cast<HoverBox *>(ui::hover_popup->tile.get());
          hover_box->clear();
          hover_box->text = clean_text(hover_text);
          hover_box->is_markup = is_markup;
          message.editor->hover_active = true;
        }
      }
    };
    lsp->send(std::move(message));
  }
}

void Editor::handle_click(KeyEvent event, Coord size) {
  focused_window = this;
  this->size = size;
  static std::chrono::steady_clock::time_point last_click_time =
      std::chrono::steady_clock::now();
  static uint32_t click_count = 0;
  static Coord last_click_pos = {UINT32_MAX, UINT32_MAX};
  if (event.key_type == KEY_MOUSE) {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - last_click_time)
                        .count();
    switch (event.mouse_state) {
    case SCROLL:
      switch (event.mouse_direction) {
      case SCROLL_UP:
        this->scroll_up(4);
        this->ensure_cursor();
        break;
      case SCROLL_DOWN:
        this->scroll_down(4);
        this->ensure_cursor();
        break;
      case SCROLL_LEFT:
        this->cursor_left(10);
        break;
      case SCROLL_RIGHT:
        this->cursor_right(10);
        break;
      }
      break;
    case PRESS:
      if (event.mouse_button == LEFT_BTN) {
        Coord cur_pos = {event.mouse_x, event.mouse_y};
        if (duration < 250 && last_click_pos == cur_pos)
          click_count++;
        else
          click_count = 1;
        last_click_time = now;
        last_click_pos = cur_pos;
        Coord p = this->click_coord(event.mouse_x, event.mouse_y);
        if (p.row == UINT32_MAX && p.col == UINT32_MAX)
          return;
        this->cursor_preffered = UINT32_MAX;
        if (click_count == 1) {
          this->cursor = p;
          this->selection = p;
          if (mode == SELECT) {
            mode = NORMAL;
            this->selection_active = false;
          }
        } else if (click_count == 2) {
          uint32_t prev_col, next_col;
          this->word_boundaries(this->cursor, &prev_col, &next_col, nullptr,
                                nullptr);
          if (this->cursor < this->selection)
            this->cursor = {this->cursor.row, prev_col};
          else
            this->cursor = {this->cursor.row, next_col};
          this->cursor_preffered = UINT32_MAX;
          this->selection_type = WORD;
          mode = SELECT;
          this->selection_active = true;
        } else if (click_count >= 3) {
          if (this->cursor < this->selection) {
            this->cursor = {p.row, 0};
          } else {
            uint32_t line_len;
            LineIterator *it = begin_l_iter(this->root, p.row);
            char *line = next_line(it, &line_len);
            if (!line)
              return;
            if (line_len > 0 && line[line_len - 1] == '\n')
              line_len--;
            free(it->buffer);
            free(it);
            this->cursor = {p.row, line_len};
          }
          this->cursor_preffered = UINT32_MAX;
          this->selection_type = LINE;
          mode = SELECT;
          this->selection_active = true;
          click_count = 3;
        }
      }
      break;
    case DRAG:
      if (event.mouse_button == LEFT_BTN) {
        Coord p = this->click_coord(event.mouse_x, event.mouse_y);
        if (p.row == UINT32_MAX && p.col == UINT32_MAX)
          return;
        this->cursor_preffered = UINT32_MAX;
        mode = SELECT;
        if (!this->selection_active) {
          this->selection_active = true;
          this->selection_type = CHAR;
        }
        uint32_t prev_col, next_col, line_len;
        switch (this->selection_type) {
        case CHAR:
          this->cursor = p;
          break;
        case WORD:
          this->word_boundaries(p, &prev_col, &next_col, nullptr, nullptr);
          if (this->cursor < this->selection)
            this->cursor = {p.row, prev_col};
          else
            this->cursor = {p.row, next_col};
          break;
        case LINE:
          if (this->cursor < this->selection) {
            this->cursor = {p.row, 0};
          } else {
            LineIterator *it = begin_l_iter(this->root, p.row);
            char *line = next_line(it, &line_len);
            if (!line)
              return;
            if (line_len > 0 && line[line_len - 1] == '\n')
              line_len--;
            free(it->buffer);
            free(it);
            this->cursor = {p.row, line_len};
          }
          break;
        }
      }
      break;
    case RELEASE:
      if (event.mouse_button == LEFT_BTN)
        if (this->cursor.row == this->selection.row &&
            this->cursor.col == this->selection.col) {
          mode = NORMAL;
          this->selection_active = false;
        }
      break;
    }
  }
}

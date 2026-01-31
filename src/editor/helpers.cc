#include "editor/helpers.h"
#include "editor/editor.h"
#include "io/sysio.h"
#include "lsp/lsp.h"
#include "main.h"
#include "utils/utils.h"

void cut(Editor *editor) {
  if (mode != SELECT)
    return;
  Coord start;
  uint32_t len;
  char *text = get_selection(editor, &len, &start);
  ruby_copy(text, len);
  len = count_clusters(text, len, 0, len);
  edit_erase(editor, start, len);
  free(text);
  editor->selection_active = false;
}

void copy(Editor *editor) {
  if (mode != SELECT)
    return;
  uint32_t len;
  char *text = get_selection(editor, &len, nullptr);
  ruby_copy(text, len);
  free(text);
  editor->selection_active = false;
}

void paste(Editor *editor) {
  if (mode == NORMAL) {
    std::string text = ruby_paste();
    if (text.empty())
      return;
    insert_str(editor, (char *)text.c_str(), text.length());
  } else if (mode == SELECT) {
    std::string text = ruby_paste();
    if (!text.empty()) {
      Coord start, end;
      selection_bounds(editor, &start, &end);
      uint32_t start_byte =
          line_to_byte(editor->root, start.row, nullptr) + start.col;
      uint32_t end_byte =
          line_to_byte(editor->root, end.row, nullptr) + end.col;
      edit_erase(editor, start, end_byte - start_byte);
      edit_insert(editor, editor->cursor, (char *)text.c_str(), text.length());
    }
    editor->selection_active = false;
  }
}

void insert_str(Editor *editor, char *c, uint32_t len) {
  if (c) {
    edit_insert(editor, editor->cursor, c, len);
    uint32_t grapheme_len = count_clusters(c, len, 0, len);
    cursor_right(editor, grapheme_len);
  }
}

void indent_current_line(Editor *editor) {
  Coord start = editor->cursor;
  uint32_t delta = editor->indents.indent_line(editor->cursor.row);
  editor->cursor.col = start.col + delta;
  editor->cursor.row = start.row;
}

void dedent_current_line(Editor *editor) {
  Coord start = editor->cursor;
  uint32_t delta = editor->indents.dedent_line(editor->cursor.row);
  editor->cursor.col = MAX((int64_t)start.col - delta, 0);
  editor->cursor.row = start.row;
}

void insert_char(Editor *editor, char c) {
  uint32_t col = editor->cursor.col;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
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
      cursor_right(editor, 1);
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
      edit_insert(editor, editor->cursor, pair, 2);
      cursor_right(editor, 1);
    } else {
      edit_insert(editor, editor->cursor, &c, 1);
      cursor_right(editor, 1);
    }
    if (editor->lsp && editor->lsp->allow_formatting_on_type) {
      for (char ch : editor->lsp->format_chars) {
        if (ch == c) {
          LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
          if (!it)
            return;
          uint32_t len;
          char *line = next_line(it, &len);
          if (!line) {
            free(it->buffer);
            free(it);
            return;
          }
          uint32_t col = utf8_offset_to_utf16(line, len, editor->cursor.col);
          free(it->buffer);
          free(it);
          int version = editor->lsp_version;
          json message = {
              {"jsonrpc", "2.0"},
              {"method", "textDocument/onTypeFormatting"},
              {"params",
               {{"textDocument", {{"uri", editor->uri}}},
                {"position",
                 {{"line", editor->cursor.row}, {"character", col}}},
                {"ch", std::string(1, c)},
                {"options",
                 {{"tabSize", 2},
                  {"insertSpaces", true},
                  {"trimTrailingWhitespace", true},
                  {"trimFinalNewlines", true}}}}}};
          LSPPending *pending = new LSPPending();
          pending->editor = editor;
          pending->method = "textDocument/onTypeFormatting";
          pending->callback = [version](Editor *editor, std::string,
                                        json message) {
            if (version != editor->lsp_version)
              return;
            auto &edits = message["result"];
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
                utf8_normalize_edit(editor, &t_edit);
                t_edits.push_back(t_edit);
              }
              apply_lsp_edits(editor, t_edits, false);
              ensure_scroll(editor);
            }
          };
          lsp_send(editor->lsp, message, pending);
          break;
        }
      }
    }
  }
}

void normal_mode(Editor *editor) {
  Coord prev_pos = editor->cursor;
  mode = NORMAL;
  cursor_left(editor, 1);
  if (prev_pos.row != editor->cursor.row)
    cursor_right(editor, 1);
}

void backspace_edit(Editor *editor) {
  Coord prev_pos = editor->cursor;
  if (prev_pos.col > 0)
    prev_pos.col--;
  LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
  if (!it)
    return;
  char *line = next_line(it, nullptr);
  char prev_char = line[prev_pos.col];
  char next_char = line[editor->cursor.col];
  free(it->buffer);
  free(it);
  bool is_pair = (prev_char == '{' && next_char == '}') ||
                 (prev_char == '(' && next_char == ')') ||
                 (prev_char == '[' && next_char == ']') ||
                 (prev_char == '"' && next_char == '"') ||
                 (prev_char == '\'' && next_char == '\'');
  if (is_pair) {
    edit_erase(editor, editor->cursor, 1);
    edit_erase(editor, prev_pos, 1);
  } else {
    edit_erase(editor, editor->cursor, -1);
  }
}

void delete_prev_word(Editor *editor) {
  uint32_t prev_col_byte, prev_col_cluster;
  word_boundaries(editor, editor->cursor, &prev_col_byte, nullptr,
                  &prev_col_cluster, nullptr);
  if (prev_col_byte == editor->cursor.col)
    edit_erase(editor, editor->cursor, -1);
  else
    edit_erase(editor, editor->cursor, -(int64_t)prev_col_cluster);
}

void delete_next_word(Editor *editor) {
  uint32_t next_col_byte, next_col_cluster;
  word_boundaries(editor, editor->cursor, nullptr, &next_col_byte, nullptr,
                  &next_col_cluster);
  if (next_col_byte == editor->cursor.col)
    edit_erase(editor, editor->cursor, 1);
  else
    edit_erase(editor, editor->cursor, next_col_cluster);
}

void clear_hooks_at_line(Editor *editor, uint32_t line) {
  for (uint8_t i = 0; i < 94; i++)
    if (editor->hooks[i] == line + 1) {
      editor->hooks[i] = 0;
      break;
    }
}

void cursor_prev_word(Editor *editor) {
  uint32_t prev_col;
  word_boundaries(editor, editor->cursor, &prev_col, nullptr, nullptr, nullptr);
  editor->cursor_preffered = UINT32_MAX;
  if (prev_col == editor->cursor.col)
    cursor_left(editor, 1);
  else
    editor->cursor = {editor->cursor.row, prev_col};
}

void cursor_next_word(Editor *editor) {
  uint32_t next_col;
  word_boundaries(editor, editor->cursor, nullptr, &next_col, nullptr, nullptr);
  editor->cursor_preffered = UINT32_MAX;
  if (next_col == editor->cursor.col)
    cursor_right(editor, 1);
  else
    editor->cursor = {editor->cursor.row, next_col};
}

void select_all(Editor *editor) {
  if (editor->root->line_count > 0) {
    editor->cursor.row = editor->root->line_count - 1;
    LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
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
    editor->cursor.col = line_len;
    editor->cursor_preffered = UINT32_MAX;
    mode = SELECT;
    editor->selection_active = true;
    editor->selection = {0, 0};
    editor->selection_type = LINE;
  }
}

void fetch_lsp_hover(Editor *editor) {
  if (editor->lsp && editor->lsp->allow_hover) {
    LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line) {
      free(it->buffer);
      free(it);
      return;
    }
    uint32_t col = utf8_offset_to_utf16(line, line_len, editor->cursor.col);
    free(it->buffer);
    free(it);
    json hover_request = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/hover"},
        {"params",
         {{"textDocument", {{"uri", editor->uri}}},
          {"position", {{"line", editor->cursor.row}, {"character", col}}}}}};
    LSPPending *pending = new LSPPending();
    pending->editor = editor;
    pending->method = "textDocument/hover";
    pending->callback = [](Editor *editor, std::string, json hover) {
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
          editor->hover.clear();
          editor->hover.text = clean_text(hover_text);
          editor->hover.is_markup = is_markup;
          editor->hover.render_first();
          editor->hover_active = true;
        }
      }
    };
    lsp_send(editor->lsp, hover_request, pending);
  }
}

void handle_mouse(Editor *editor, KeyEvent event) {
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
        scroll_up(editor, 4);
        ensure_cursor(editor);
        break;
      case SCROLL_DOWN:
        scroll_down(editor, 4);
        ensure_cursor(editor);
        break;
      case SCROLL_LEFT:
        cursor_left(editor, 10);
        break;
      case SCROLL_RIGHT:
        cursor_right(editor, 10);
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
        Coord p = editor_hit_test(editor, event.mouse_x, event.mouse_y);
        if (p.row == UINT32_MAX && p.col == UINT32_MAX)
          return;
        editor->cursor_preffered = UINT32_MAX;
        if (click_count == 1) {
          editor->cursor = p;
          editor->selection = p;
          if (mode == SELECT) {
            mode = NORMAL;
            editor->selection_active = false;
          }
        } else if (click_count == 2) {
          uint32_t prev_col, next_col;
          word_boundaries(editor, editor->cursor, &prev_col, &next_col, nullptr,
                          nullptr);
          if (editor->cursor < editor->selection)
            editor->cursor = {editor->cursor.row, prev_col};
          else
            editor->cursor = {editor->cursor.row, next_col};
          editor->cursor_preffered = UINT32_MAX;
          editor->selection_type = WORD;
          mode = SELECT;
          editor->selection_active = true;
        } else if (click_count >= 3) {
          if (editor->cursor < editor->selection) {
            editor->cursor = {p.row, 0};
          } else {
            uint32_t line_len;
            LineIterator *it = begin_l_iter(editor->root, p.row);
            char *line = next_line(it, &line_len);
            if (!line)
              return;
            if (line_len > 0 && line[line_len - 1] == '\n')
              line_len--;
            free(it->buffer);
            free(it);
            editor->cursor = {p.row, line_len};
          }
          editor->cursor_preffered = UINT32_MAX;
          editor->selection_type = LINE;
          mode = SELECT;
          editor->selection_active = true;
          click_count = 3;
        }
      }
      break;
    case DRAG:
      if (event.mouse_button == LEFT_BTN) {
        Coord p = editor_hit_test(editor, event.mouse_x, event.mouse_y);
        if (p.row == UINT32_MAX && p.col == UINT32_MAX)
          return;
        editor->cursor_preffered = UINT32_MAX;
        mode = SELECT;
        if (!editor->selection_active) {
          editor->selection_active = true;
          editor->selection_type = CHAR;
        }
        uint32_t prev_col, next_col, line_len;
        switch (editor->selection_type) {
        case CHAR:
          editor->cursor = p;
          break;
        case WORD:
          word_boundaries(editor, p, &prev_col, &next_col, nullptr, nullptr);
          if (editor->cursor < editor->selection)
            editor->cursor = {p.row, prev_col};
          else
            editor->cursor = {p.row, next_col};
          break;
        case LINE:
          if (editor->cursor < editor->selection) {
            editor->cursor = {p.row, 0};
          } else {
            LineIterator *it = begin_l_iter(editor->root, p.row);
            char *line = next_line(it, &line_len);
            if (!line)
              return;
            if (line_len > 0 && line[line_len - 1] == '\n')
              line_len--;
            free(it->buffer);
            free(it);
            editor->cursor = {p.row, line_len};
          }
          break;
        }
      }
      break;
    case RELEASE:
      if (event.mouse_button == LEFT_BTN)
        if (editor->cursor.row == editor->selection.row &&
            editor->cursor.col == editor->selection.col) {
          mode = NORMAL;
          editor->selection_active = false;
        }
      break;
    }
  }
}

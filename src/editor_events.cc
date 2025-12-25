#include "../include/editor.h"
#include "../include/main.h"
#include "../include/ts.h"
#include <cstdint>
#include <sys/ioctl.h>

void handle_editor_event(Editor *editor, KeyEvent event) {
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
        scroll_up(editor, 10);
        ensure_cursor(editor);
        break;
      case SCROLL_DOWN:
        scroll_down(editor, 10);
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
  if (event.key_type == KEY_SPECIAL) {
    switch (event.special_modifier) {
    case 0:
      switch (event.special_key) {
      case KEY_DOWN:
        cursor_down(editor, 1);
        break;
      case KEY_UP:
        cursor_up(editor, 1);
        break;
      case KEY_LEFT:
        cursor_left(editor, 1);
        break;
      case KEY_RIGHT:
        cursor_right(editor, 1);
        break;
      }
      break;
    case CNTRL:
      uint32_t prev_col, next_col;
      word_boundaries(editor, editor->cursor, &prev_col, &next_col, nullptr,
                      nullptr);
      switch (event.special_key) {
      case KEY_DOWN:
        cursor_down(editor, 5);
        break;
      case KEY_UP:
        cursor_up(editor, 5);
        break;
      case KEY_LEFT:
        editor->cursor_preffered = UINT32_MAX;
        if (prev_col == editor->cursor.col)
          cursor_left(editor, 1);
        else
          editor->cursor = {editor->cursor.row, prev_col};
        break;
      case KEY_RIGHT:
        editor->cursor_preffered = UINT32_MAX;
        if (next_col == editor->cursor.col)
          cursor_right(editor, 1);
        else
          editor->cursor = {editor->cursor.row, next_col};
        break;
      }
      break;
    case ALT:
      switch (event.special_key) {
      case KEY_DOWN:
        move_line_down(editor);
        break;
      case KEY_UP:
        move_line_up(editor);
        break;
      case KEY_LEFT:
        cursor_left(editor, 8);
        break;
      case KEY_RIGHT:
        cursor_right(editor, 8);
        break;
      }
      break;
    }
  }
  switch (mode) {
  case NORMAL:
    if (event.key_type == KEY_CHAR && event.len == 1) {
      Coord start = editor->cursor;
      switch (event.c[0]) {
      case 'u':
        if (editor->root->line_count > 0) {
          editor->cursor.row = editor->root->line_count - 1;
          LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
          if (!it)
            break;
          uint32_t line_len;
          char *line = next_line(it, &line_len);
          if (!line)
            break;
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
        break;
      case 'a':
        mode = INSERT;
        cursor_right(editor, 1);
        if (start.row != editor->cursor.row)
          cursor_left(editor, 1);
        break;
      case 'i':
        mode = INSERT;
        break;
      case 'n':
        mode = JUMPER;
        editor->jumper_set = true;
        break;
      case 'm':
        mode = JUMPER;
        editor->jumper_set = false;
        break;
      case 'N':
        for (uint8_t i = 0; i < 94; i++)
          if (editor->hooks[i] == editor->cursor.row + 1) {
            editor->hooks[i] = 0;
            break;
          }
        break;
      case 's':
      case 'v':
        mode = SELECT;
        editor->selection_active = true;
        editor->selection = editor->cursor;
        editor->selection_type = CHAR;
        break;
      case ';':
      case ':':
        mode = RUNNER;
        break;
      case 0x7F:
        cursor_left(editor, 1);
        break;
      case ' ':
        cursor_right(editor, 1);
        break;
      case '\r':
      case '\n':
        cursor_down(editor, 1);
        break;
      case '\\':
      case '|':
        cursor_up(editor, 1);
        break;
      case CTRL('d'):
        scroll_down(editor, 1);
        ensure_cursor(editor);
        break;
      case CTRL('u'):
        scroll_up(editor, 1);
        ensure_cursor(editor);
        break;
      case '>':
      case '.':
        indent_line(editor, editor->cursor.row);
        break;
      case '<':
      case ',':
        dedent_line(editor, editor->cursor.row);
        break;
      case CTRL('s'):
        save_file(editor);
        break;
      case 'p':
        uint32_t len;
        char *text = get_from_clipboard(&len);
        if (text) {
          edit_insert(editor, editor->cursor, text, len);
          uint32_t grapheme_len = count_clusters(text, len, 0, len);
          cursor_right(editor, grapheme_len);
          free(text);
        }
        break;
      }
    }
    break;
  case INSERT:
    if (event.key_type == KEY_CHAR) {
      if (event.len == 1) {
        if (event.c[0] == '\t') {
          edit_insert(editor, editor->cursor, (char *)"  ", 2);
          cursor_right(editor, 2);
        } else if (event.c[0] == '\n' || event.c[0] == '\r') {
          uint32_t line_len = 0;
          LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
          char *line = next_line(it, &line_len);
          bool closing = false;
          if (line && line_len > 0 && line[line_len - 1] == '\n')
            line_len--;
          uint32_t indent = get_indent(editor, editor->cursor);
          if (line) {
            if (indent == 0)
              indent = leading_indent(line, line_len);
            closing = closing_after_cursor(line, line_len, editor->cursor.col);
          }
          free(it->buffer);
          free(it);
          uint32_t closing_indent =
              indent >= INDENT_WIDTH ? indent - INDENT_WIDTH : 0;
          std::string insert_text("\n");
          insert_text.append(indent, ' ');
          Coord new_cursor = {editor->cursor.row + 1, indent};
          if (closing) {
            insert_text.push_back('\n');
            insert_text.append(closing_indent, ' ');
          }
          edit_insert(editor, editor->cursor, insert_text.data(),
                      insert_text.size());
          editor->cursor = new_cursor;
          editor->cursor_preffered = UINT32_MAX;
        } else if (event.c[0] == CTRL('W')) {
          uint32_t prev_col_byte, prev_col_cluster;
          word_boundaries(editor, editor->cursor, &prev_col_byte, nullptr,
                          &prev_col_cluster, nullptr);
          if (prev_col_byte == editor->cursor.col)
            edit_erase(editor, editor->cursor, -1);
          else
            edit_erase(editor, editor->cursor, -(int64_t)prev_col_cluster);
        } else if (isprint((unsigned char)(event.c[0]))) {
          char c = event.c[0];
          uint32_t col = editor->cursor.col;
          LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
          uint32_t len;
          char *line = next_line(it, &len);
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
          }
        } else if (event.c[0] == 0x7F || event.c[0] == 0x08) {
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
        } else if (event.c[0] == 0x1B) {
          Coord prev_pos = editor->cursor;
          mode = NORMAL;
          cursor_left(editor, 1);
          if (prev_pos.row != editor->cursor.row)
            cursor_right(editor, 1);
        }
      } else if (event.len > 1) {
        edit_insert(editor, editor->cursor, event.c, event.len);
        cursor_right(editor, 1);
      }
    } else if (event.key_type == KEY_SPECIAL &&
               event.special_key == KEY_DELETE) {
      switch (event.special_modifier) {
      case 0:
        edit_erase(editor, editor->cursor, 1);
        break;
      case CNTRL:
        uint32_t next_col_byte, next_col_cluster;
        word_boundaries(editor, editor->cursor, nullptr, &next_col_byte,
                        nullptr, &next_col_cluster);
        if (next_col_byte == editor->cursor.col)
          edit_erase(editor, editor->cursor, 1);
        else
          edit_erase(editor, editor->cursor, next_col_cluster);
        break;
      }
    } else if (event.key_type == KEY_PASTE) {
      if (event.c) {
        edit_insert(editor, editor->cursor, event.c, event.len);
        uint32_t grapheme_len =
            count_clusters(event.c, event.len, 0, event.len);
        cursor_right(editor, grapheme_len);
      }
    }
    break;
  case SELECT:
    if (event.key_type == KEY_CHAR && event.len == 1) {
      uint32_t len;
      char *text;
      Coord start;
      switch (event.c[0]) {
      case 'f':
        if (editor->cursor.row != editor->selection.row) {
          uint32_t start = MIN(editor->cursor.row, editor->selection.row);
          uint32_t end = MAX(editor->cursor.row, editor->selection.row);
          add_fold(editor, start, end);
        }
        cursor_left(editor, 1);
        cursor_down(editor, 1);
        editor->selection_active = false;
        mode = NORMAL;
        break;
      case 0x1B:
      case 's':
      case 'v':
        editor->selection_active = false;
        mode = NORMAL;
        break;
      case 'y':
        text = get_selection(editor, &len, nullptr);
        copy_to_clipboard(text, len);
        free(text);
        editor->selection_active = false;
        mode = NORMAL;
        break;
      case 'x':
        text = get_selection(editor, &len, &start);
        copy_to_clipboard(text, len);
        len = count_clusters(text, len, 0, len);
        edit_erase(editor, start, len);
        free(text);
        editor->selection_active = false;
        mode = NORMAL;
        break;
      case 'p':
        text = get_from_clipboard(&len);
        if (text) {
          Coord start, end;
          if (editor->cursor >= editor->selection) {
            start = editor->selection;
            end = move_right(editor, editor->cursor, 1);
          } else {
            start = editor->cursor;
            end = move_right(editor, editor->selection, 1);
          }
          uint32_t start_byte =
              line_to_byte(editor->root, start.row, nullptr) + start.col;
          uint32_t end_byte =
              line_to_byte(editor->root, end.row, nullptr) + end.col;
          edit_erase(editor, start, end_byte - start_byte);
          edit_insert(editor, editor->cursor, text, len);
          free(text);
        }
        editor->selection_active = false;
        mode = NORMAL;
        break;
      }
    }
    break;
  case JUMPER:
    if (event.key_type == KEY_CHAR && event.len == 1 &&
        (event.c[0] >= '!' && event.c[0] <= '~')) {
      if (editor->jumper_set) {
        for (uint8_t i = 0; i < 94; i++)
          if (editor->hooks[i] == editor->cursor.row + 1) {
            editor->hooks[i] = 0;
            break;
          }
        editor->hooks[event.c[0] - '!'] = editor->cursor.row + 1;
      } else {
        uint32_t line = editor->hooks[event.c[0] - '!'];
        if (line > 0) {
          if (line_is_folded(editor->folds, --line))
            break;
          editor->cursor = {line, 0};
          editor->cursor_preffered = UINT32_MAX;
        }
      }
    }
    mode = NORMAL;
    break;
  case RUNNER:
    if (event.key_type == KEY_CHAR && event.len == 1) {
      switch (event.c[0]) {
      case 0x1B:
        mode = NORMAL;
        break;
      }
    }
    break;
  }
  ensure_scroll(editor);
  if ((event.key_type == KEY_CHAR || event.key_type == KEY_PASTE) && event.c)
    free(event.c);
}

static Highlight HL_UNDERLINE = {0, 0, 1 << 2, 100};

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
  if (next_col - prev_col > 0 && next_col - prev_col < 256 - 4) {
    std::shared_lock lock(editor->knot_mtx);
    uint32_t offset = line_to_byte(editor->root, editor->cursor.row, nullptr);
    char *word = read(editor->root, offset + prev_col, next_col - prev_col);
    if (word) {
      char buf[256];
      snprintf(buf, sizeof(buf), "\\b%s\\b", word);
      std::vector<std::pair<size_t, size_t>> results =
          search_rope(editor->root, buf);
      std::unique_lock lock(editor->def_spans.mtx);
      editor->def_spans.spans.clear();
      for (const auto &match : results) {
        Span s;
        s.start = match.first;
        s.end = match.first + match.second;
        s.hl = &HL_UNDERLINE;
        editor->def_spans.spans.push_back(s);
      }
      std::sort(editor->def_spans.spans.begin(), editor->def_spans.spans.end());
      lock.unlock();
      free(word);
    }
  } else {
    std::unique_lock lock(editor->def_spans.mtx);
    editor->def_spans.spans.clear();
    lock.unlock();
  }
}

void editor_lsp_handle(Editor *editor, json msg) {
  if (msg.contains("method") &&
      msg["method"] == "textDocument/publishDiagnostics") {
    std::unique_lock lock(editor->v_mtx);
    editor->warnings.clear();
    json diagnostics = msg["params"]["diagnostics"];
    for (size_t i = 0; i < diagnostics.size(); i++) {
      json d = diagnostics[i];
      VWarn w;
      w.line = d["range"]["start"]["line"];
      std::string text = d["message"].get<std::string>();
      auto pos = text.find('\n');
      w.text = (pos == std::string::npos) ? text : text.substr(0, pos);
      w.type = d["severity"].get<int>();
      editor->warnings.push_back(w);
    }
    std::sort(editor->warnings.begin(), editor->warnings.end());
  }
}

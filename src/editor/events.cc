#include "editor/editor.h"
#include "editor/helpers.h"
#include "io/sysio.h"
#include "main.h"
#include "utils/utils.h"

void handle_editor_event(Editor *editor, KeyEvent event) {
  uint8_t old_mode = mode;
  if (editor->hover_active)
    editor->hover_active = false;
  handle_mouse(editor, event);
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
      switch (event.special_key) {
      case KEY_DOWN:
        cursor_down(editor, 5);
        break;
      case KEY_UP:
        cursor_up(editor, 5);
        break;
      case KEY_LEFT:
        cursor_prev_word(editor);
      case KEY_RIGHT:
        cursor_next_word(editor);
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
      switch (event.c[0]) {
      case 'u':
        select_all(editor);
        break;
      case CTRL('h'):
        editor->hover.scroll(-1);
        editor->hover_active = true;
        break;
      case CTRL('l'):
        editor->hover.scroll(1);
        editor->hover_active = true;
        break;
      case 'h':
        fetch_lsp_hover(editor);
        break;
      case 'a': {
        mode = INSERT;
        Coord start = editor->cursor;
        cursor_right(editor, 1);
        if (start.row != editor->cursor.row)
          cursor_left(editor, 1);
      } break;
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
        clear_hooks_at_line(editor, editor->cursor.row);
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
        indent_current_line(editor);
        break;
      case '<':
      case ',':
        dedent_current_line(editor);
        break;
      case CTRL('s'):
        save_file(editor);
        break;
      case 'p':
        paste(editor);
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
          editor->indents.insert_new_line(editor->cursor);
        } else if (event.c[0] == CTRL('W')) {
          delete_prev_word(editor);
        } else if (isprint((unsigned char)(event.c[0]))) {
          insert_char(editor, event.c[0]);
        } else if (event.c[0] == 0x7F || event.c[0] == 0x08) {
          backspace_edit(editor);
        } else if (event.c[0] == 0x1B) {
          normal_mode(editor);
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
        delete_next_word(editor);
        break;
      }
    } else if (event.key_type == KEY_PASTE) {
      insert_str(editor, event.c, event.len);
    }
    break;
  case SELECT:
    if (event.key_type == KEY_CHAR && event.len == 1) {
      switch (event.c[0]) {
      case 0x1B:
      case 's':
      case 'v':
        editor->selection_active = false;
        mode = NORMAL;
        break;
      case 'y':
        copy(editor);
        mode = NORMAL;
        break;
      case 'x':
        cut(editor);
        mode = NORMAL;
        break;
      case 'p':
        paste(editor);
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
        uint32_t line = editor->hooks[event.c[0] - '!'] - 1;
        if (line > 0) {
          editor->cursor = {line, 0};
          editor->cursor_preffered = UINT32_MAX;
        }
      }
    }
    mode = NORMAL;
    break;
  }
  if (old_mode == mode || mode != INSERT)
    handle_completion(editor, event);
  ensure_scroll(editor);
  if ((event.key_type == KEY_CHAR || event.key_type == KEY_PASTE) && event.c)
    free(event.c);
}

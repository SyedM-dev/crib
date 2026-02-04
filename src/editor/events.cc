#include "editor/editor.h"
#include "extentions/hover.h"
#include "io/sysio.h"
#include "main.h"
#include "utils/utils.h"

void Editor::handle_event(KeyEvent event) {
  uint8_t old_mode = mode;
  if (this->hover_active)
    this->hover_active = false;
  if (event.key_type == KEY_SPECIAL) {
    switch (event.special_modifier) {
    case 0:
      switch (event.special_key) {
      case KEY_DOWN:
        this->cursor_down(1);
        break;
      case KEY_UP:
        this->cursor_up(1);
        break;
      case KEY_LEFT:
        this->cursor_left(1);
        break;
      case KEY_RIGHT:
        this->cursor_right(1);
        break;
      }
      break;
    case CNTRL:
      switch (event.special_key) {
      case KEY_DOWN:
        this->cursor_down(5);
        break;
      case KEY_UP:
        this->cursor_up(5);
        break;
      case KEY_LEFT:
        this->cursor_prev_word();
      case KEY_RIGHT:
        this->cursor_next_word();
        break;
      }
      break;
    case ALT:
      switch (event.special_key) {
      case KEY_DOWN:
        this->move_line_down();
        break;
      case KEY_UP:
        this->move_line_up();
        break;
      case KEY_LEFT:
        this->cursor_left(8);
        break;
      case KEY_RIGHT:
        this->cursor_right(8);
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
        this->select_all();
        break;
      case CTRL('h'):
        static_cast<HoverBox *>(ui::hover_popup->tile.get())->scroll(-1);
        this->hover_active = true;
        break;
      case CTRL('l'):
        static_cast<HoverBox *>(ui::hover_popup->tile.get())->scroll(1);
        this->hover_active = true;
        break;
      case 'h':
        this->fetch_lsp_hover();
        break;
      case 'a': {
        mode = INSERT;
        Coord start = this->cursor;
        this->cursor_right(1);
        if (start.row != this->cursor.row)
          this->cursor_left(1);
      } break;
      case 'i':
        mode = INSERT;
        break;
      case 'n':
        mode = JUMPER;
        this->jumper_set = true;
        break;
      case 'm':
        mode = JUMPER;
        this->jumper_set = false;
        break;
      case 'N':
        this->clear_hooks_at_line(this->cursor.row);
        break;
      case 's':
      case 'v':
        mode = SELECT;
        this->selection_active = true;
        this->selection = this->cursor;
        this->selection_type = CHAR;
        break;
      case ';':
      case ':':
        mode = RUNNER;
        break;
      case 0x7F:
        this->cursor_left(1);
        break;
      case ' ':
        this->cursor_right(1);
        break;
      case '\r':
      case '\n':
        this->cursor_down(1);
        break;
      case '\\':
      case '|':
        this->cursor_up(1);
        break;
      case CTRL('d'):
        this->scroll_down(1);
        this->ensure_cursor();
        break;
      case CTRL('u'):
        this->scroll_up(1);
        this->ensure_cursor();
        break;
      case '>':
      case '.':
        this->indent_current_line();
        break;
      case '<':
      case ',':
        this->dedent_current_line();
        break;
      case CTRL('s'):
        this->save();
        break;
      case 'p':
        this->paste();
        break;
      }
    }
    break;
  case INSERT:
    if (event.key_type == KEY_CHAR) {
      if (event.len == 1) {
        if (event.c[0] == '\t') {
          this->indents.insert_tab(this->cursor);
        } else if (event.c[0] == '\n' || event.c[0] == '\r') {
          this->indents.insert_new_line(this->cursor);
        } else if (event.c[0] == CTRL('W')) {
          this->delete_prev_word();
        } else if (isprint((unsigned char)(event.c[0]))) {
          this->insert_char(event.c[0]);
        } else if (event.c[0] == 0x7F || event.c[0] == 0x08) {
          this->backspace_edit();
        } else if (event.c[0] == 0x1B) {
          this->normal_mode();
        }
      } else if (event.len > 1) {
        this->edit_insert(this->cursor, event.c, event.len);
        this->cursor_right(1);
      }
    } else if (event.key_type == KEY_SPECIAL &&
               event.special_key == KEY_DELETE) {
      switch (event.special_modifier) {
      case 0:
        this->edit_erase(this->cursor, 1);
        break;
      case CNTRL:
        this->delete_next_word();
        break;
      }
    } else if (event.key_type == KEY_PASTE) {
      this->insert_str(event.c, event.len);
    }
    break;
  case SELECT:
    if (event.key_type == KEY_CHAR && event.len == 1) {
      switch (event.c[0]) {
      case 0x1B:
      case 's':
      case 'v':
        this->selection_active = false;
        mode = NORMAL;
        break;
      case 'y':
        this->copy();
        mode = NORMAL;
        break;
      case 'x':
        this->cut();
        mode = NORMAL;
        break;
      case 'p':
        this->paste();
        mode = NORMAL;
        break;
      case '<':
      case ',':
        this->dedent_selection();
        break;
      case '>':
      case '.':
        this->indent_selection();
        break;
      }
    }
    break;
  case JUMPER:
    if (event.key_type == KEY_CHAR && event.len == 1 &&
        (event.c[0] >= '!' && event.c[0] <= '~')) {
      if (this->jumper_set) {
        for (uint8_t i = 0; i < 94; i++)
          if (this->hooks[i] == this->cursor.row + 1) {
            this->hooks[i] = 0;
            break;
          }
        this->hooks[event.c[0] - '!'] = this->cursor.row + 1;
      } else {
        uint32_t line = this->hooks[event.c[0] - '!'] - 1;
        if (line > 0) {
          this->cursor = {line, 0};
          this->cursor_preffered = UINT32_MAX;
        }
      }
    }
    mode = NORMAL;
    break;
  }
  // if (old_mode == mode || mode != INSERT)
  //   this->completion.handle(event);
  this->ensure_scroll();
}

#include "ui/bar.h"
#include "io/sysio.h"
#include "main.h"

void Bar::render() {
  Editor *editor = editors[current_editor];
  uint32_t row = screen.row - 2;
  uint32_t col = 0;
  uint32_t width = screen.col;
  uint32_t color = 0;
  uint32_t black = 0x0b0e14;
  uint32_t grey = 0x33363c;
  uint32_t dark_grey = 0x24272d;
  uint32_t name_color = 0xced4df;
  uint32_t lang_color = editor->lang.color;
  const char *symbol = "󱓧 ";
  const char *name = "EDITOR";
  switch (mode) {
  case NORMAL:
    color = 0x82AAFF;
    symbol = " ";
    name = "NORMAL";
    break;
  case INSERT:
    color = 0xFF8F40;
    symbol = "󱓧 ";
    name = "INSERT";
    break;
  case SELECT:
    color = 0x9ADE7A;
    symbol = "󱩧 ";
    name = "SELECT";
    break;
  case RUNNER:
    color = 0xFFD700;
    symbol = " ";
    name = "RUNNER";
    break;
  case JUMPER:
    color = 0xF29CC3;
    symbol = " ";
    name = "JUMPER";
    break;
  }
  update(row, col, " ", black, color, CF_BOLD);
  update(row, ++col, symbol, black, color, CF_BOLD);
  update(row, ++col, "\x1b", black, color, CF_BOLD);
  update(row, ++col, " ", black, color, CF_BOLD);
  for (uint32_t i = 0; i < 6; i++)
    update(row, ++col, {name[i], 0}, black, color, CF_BOLD);
  update(row, ++col, " ", black, color, CF_BOLD);
  update(row, ++col, "◗", color, grey, CF_BOLD);
  update(row, ++col, "◗", grey, dark_grey, CF_BOLD);
  update(row, ++col, " ", name_color, dark_grey, CF_BOLD);
  update(row, ++col, editor->lang.symbol, lang_color, dark_grey, 0);
  update(row, ++col, "\x1b", lang_color, dark_grey, 0);
  update(row, ++col, " ", name_color, dark_grey, CF_BOLD);
  std::string filename = filename_from_path(editor->filename);
  for (uint32_t i = 0; i < filename.length(); i++)
    update(row, ++col, {filename[i], 0}, name_color, dark_grey, CF_BOLD);
  update(row, ++col, " ", name_color, dark_grey, CF_BOLD);
  update(row, ++col, "◗", dark_grey, 1, CF_BOLD);
}

void Bar::handle(KeyEvent event) {
  if (event.key_type == KEY_CHAR && event.len == 1) {
    if (event.c[0] == 0x1B) {
      mode = NORMAL;
    } else if (event.c[0] == '\n' || event.c[0] == '\r') {
      // execute command while stripping starting `[:;]`
      mode = NORMAL;
      command = "";
    } else if (isprint((unsigned char)(event.c[0]))) {
    } else if (event.c[0] == 0x7F || event.c[0] == 0x08) { // backspace
    }
  } else if (event.key_type == KEY_SPECIAL) {
    switch (event.special_key) {
    case KEY_LEFT:
      if (command.length() > 0)
        cursor--;
      break;
    case KEY_RIGHT:
      if (cursor < command.length())
        cursor++;
      break;
    case KEY_UP:
      // TODO: history
      break;
    case KEY_DOWN:
      // TODO: history
      break;
    }
  } else if (event.key_type == KEY_PASTE) {
  } else if (event.key_type == KEY_MOUSE) {
  }
}

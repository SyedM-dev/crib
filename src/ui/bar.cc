#include "ui/bar.h"
#include "io/sysio.h"
#include "lsp/lsp.h"
#include "main.h"
#include "syntax/decl.h"

void Bar::work() {
  std::lock_guard<std::mutex> lock(mtx);
  Editor *editor = editors[current_editor];
  bar_line =
      bar_contents(mode, editor->lang.name, editor->warnings.size(),
                   editor->lsp ? editor->lsp->lsp->command : "",
                   editor->filename, editor->filename, editor->cursor.row + 1,
                   editor->root->line_count + 1, screen.col);
}

void Bar::log(std::string message) {
  std::lock_guard<std::mutex> lock(mtx);
  log_line = message;
}

void Bar::render() {
  std::lock_guard<std::mutex> lock(mtx);
  USING(LSPInstance);
  uint32_t row = screen.row - 2;
  uint32_t width = screen.col;
  std::string &line = bar_line.line;
  uint32_t i = 0;
  uint32_t col = 0;
  while (i < line.length()) {
    uint32_t cluster_len =
        grapheme_next_character_break_utf8(line.c_str() + i, line.length() - i);
    std::string cluster = line.substr(i, cluster_len);
    int width = display_width(cluster.c_str(), cluster_len);
    Highlight highlight = bar_line.get_highlight(col);
    update(row, col, cluster.c_str(), highlight.fg, highlight.bg,
           highlight.flags);
    col += width;
    i += cluster_len;
    for (int w = 1; w < width; w++)
      update(row, col - w, "\x1b", highlight.fg, highlight.bg, highlight.flags);
  }
  while (col < width)
    update(row, col++, " ", 0, 0, 0);
  col = 0;
  row++;
  if (mode == RUNNER) {
    update(row, col++, ":", 0xFFFFFF, 0, 0);
    for (char c : command)
      update(row, col++, (char[2]){c, 0}, 0xFFFFFF, 0, 0);
  } else {
    for (char c : log_line)
      update(row, col++, (char[2]){c, 0}, 0xFFFFFF, 0, 0);
  }
  while (col < width)
    update(row, col++, " ", 0, 0, 0);
}

void Bar::handle(KeyEvent event) {
  if (event.key_type == KEY_CHAR && event.len == 1) {
    if (event.c[0] == 0x1B) {
      mode = NORMAL;
    } else if (event.c[0] == '\n' || event.c[0] == '\r') {
      command = trim(command);
      if (command == "w") {
        save_file(editors[current_editor]);
      } else if (command == "q") {
        running = false;
      } else if (command == "wq") {
        save_file(editors[current_editor]);
        running = false;
      }
      mode = NORMAL;
      command = "";
    } else if (isprint((unsigned char)(event.c[0]))) {
      command += event.c[0];
    } else if (event.c[0] == 0x7F || event.c[0] == 0x08) {
      if (command.length() > 0) {
        command = command.substr(0, command.length() - 1);
      } else {
        mode = NORMAL;
        command = "";
      }
    }
  } else if (event.key_type == KEY_SPECIAL) {
    switch (event.special_key) {
    case KEY_LEFT:
      if (command.length() > 0)
        cursor--;
      break;
    case KEY_RIGHT:
      if (cursor < (uint32_t)command.length())
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

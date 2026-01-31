#include "main.h"
#include "editor/editor.h"
#include "io/sysio.h"
#include "lsp/lsp.h"
#include "scripting/decl.h"
#include "ui/bar.h"
#include "utils/utils.h"

std::atomic<bool> running{true};
Queue<KeyEvent> event_queue;
std::vector<Editor *> editors;
Bar bar;

uint8_t current_editor = 0;
std::atomic<uint8_t> mode = NORMAL;

void background_lsp() {
  while (running)
    throttle(8ms, lsp_worker);
}

inline Editor *editor_at(uint8_t x, uint8_t y) {
  for (Editor *ed : editors) {
    Coord pos = ed->position;
    Coord size = ed->size;
    if (x >= pos.col && x < pos.col + size.col && y >= pos.row &&
        y < pos.row + size.row)
      return ed;
  }
  return nullptr;
}

inline uint8_t index_of(Editor *ed) {
  for (uint8_t i = 0; i < editors.size(); i++)
    if (editors[i] == ed)
      return i;
  return 0;
}

int main(int argc, char *argv[]) {
  ruby_start();
  load_theme();
  load_languages_info();
  load_custom_highlighters();

  Coord screen = start_screen();
  const char *filename = (argc > 1) ? argv[1] : "";
  uint8_t eol = read_line_endings();
  Editor *editor =
      new_editor(filename, {0, 0}, {screen.row - 2, screen.col}, eol);
  bar.init(screen);

  if (!editor) {
    end_screen();
    fprintf(stderr, "Failed to load editor\n");
    return 1;
  }

  editors.push_back(editor);
  current_editor = editors.size() - 1;

  std::thread lsp_thread(background_lsp);

  while (running) {
    KeyEvent event = throttle(1ms, read_key);
    if (event.key_type == KEY_NONE)
      goto render;
    if (event.key_type == KEY_CHAR && event.len == 1 &&
        event.c[0] == CTRL('q')) {
      free(event.c);
      running = false;
      break;
    }
    if (mode != RUNNER) {
      if (event.key_type == KEY_MOUSE) {
        Editor *target = editor_at(event.mouse_x, event.mouse_y);
        if (target) {
          if (event.mouse_state == PRESS)
            current_editor = index_of(target);
          event.mouse_x -= target->position.col;
          event.mouse_y -= target->position.row;
          handle_editor_event(target, event);
        }
      } else {
        handle_editor_event(editors[current_editor], event);
      }
    } else {
      bar.handle(event);
    }
    if ((event.key_type == KEY_CHAR || event.key_type == KEY_PASTE) && event.c)
      free(event.c);
  render:
    throttle(4ms, editor_worker, editors[current_editor]);
    bar.work();
    bar.render();
    render_editor(editors[current_editor]);
    throttle(4ms, render);
  }

  if (lsp_thread.joinable())
    lsp_thread.join();

  end_screen();

  for (auto editor : editors)
    free_editor(editor);

  ruby_shutdown();

  return 0;
}

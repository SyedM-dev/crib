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

void input_listener(Bar bar) {
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
  render:
    render_editor(editors[current_editor]);
    bar.render();
    throttle(4ms, render);
  }
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
  Bar bar(screen);

  if (!editor) {
    end_screen();
    fprintf(stderr, "Failed to load editor\n");
    return 1;
  }

  editors.push_back(editor);
  current_editor = editors.size() - 1;

  std::thread input_thread(input_listener, bar);
  std::thread lsp_thread(background_lsp);

  while (running)
    throttle(16ms, editor_worker, editors[current_editor]);

  if (input_thread.joinable())
    input_thread.join();

  if (lsp_thread.joinable())
    lsp_thread.join();

  end_screen();

  for (auto editor : editors)
    free_editor(editor);

  std::unique_lock lk(active_lsps_mtx);
  lk.unlock();
  while (true) {
    lk.lock();
    if (active_lsps.empty())
      break;
    lk.unlock();
    throttle(16ms, lsp_worker);
  }

  ruby_shutdown();

  return 0;
}

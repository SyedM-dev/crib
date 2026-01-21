#include "main.h"
#include "editor/editor.h"
#include "io/sysio.h"
#include "lsp/lsp.h"
#include "ui/bar.h"
#include "utils/utils.h"

std::atomic<bool> running{true};
Queue<KeyEvent> event_queue;
std::vector<Editor *> editors;

uint8_t current_editor = 0;
std::atomic<uint8_t> mode = NORMAL;

void background_worker() {
  while (running)
    throttle(16ms, editor_worker, editors[current_editor]);
}

void background_lsp() {
  while (running)
    throttle(8ms, lsp_worker);
}

void input_listener() {
  while (running) {
    KeyEvent event = throttle(1ms, read_key);
    if (event.key_type == KEY_NONE)
      continue;
    if (event.key_type == KEY_CHAR && event.len == 1 &&
        event.c[0] == CTRL('q')) {
      free(event.c);
      running = false;
      return;
    }
    event_queue.push(event);
  }
}

Editor *editor_at(uint8_t x, uint8_t y) {
  for (Editor *ed : editors) {
    Coord pos = ed->position;
    Coord size = ed->size;
    if (x >= pos.col && x < pos.col + size.col && y >= pos.row &&
        y < pos.row + size.row)
      return ed;
  }
  return nullptr;
}

uint8_t index_of(Editor *ed) {
  for (uint8_t i = 0; i < editors.size(); i++)
    if (editors[i] == ed)
      return i;
  return 0;
}

int main(int argc, char *argv[]) {
  Coord screen = start_screen();
  const char *filename = (argc > 1) ? argv[1] : "";

  int state;
  VALUE result;
  result = rb_eval_string_protect("puts 'Hello, world!'", &state);

  if (state) {
    /* handle exception */
  }

  load_theme();

  Editor *editor = new_editor(filename, {0, 0}, {screen.row - 2, screen.col});
  Bar bar(screen);

  if (!editor) {
    end_screen();
    fprintf(stderr, "Failed to load editor\n");
    return 1;
  }

  editors.push_back(editor);
  current_editor = editors.size() - 1;

  std::thread input_thread(input_listener);
  std::thread work_thread(background_worker);
  std::thread lsp_thread(background_lsp);

  while (running) {
    KeyEvent event;
    while (event_queue.pop(event)) {
      if (mode != RUNNER) {
        if (event.key_type == KEY_MOUSE) {
          Editor *target = editor_at(event.mouse_x, event.mouse_y);
          if (!target)
            continue;
          if (event.mouse_state == PRESS)
            current_editor = index_of(target);
          event.mouse_x -= target->position.col;
          event.mouse_y -= target->position.row;
          handle_editor_event(target, event);
        } else {
          handle_editor_event(editors[current_editor], event);
        }
      } else {
        bar.handle(event);
      }
    }
    render_editor(editors[current_editor]);
    bar.render();
    throttle(4ms, render);
  }

  if (input_thread.joinable())
    input_thread.join();

  if (work_thread.joinable())
    work_thread.join();

  if (lsp_thread.joinable())
    lsp_thread.join();

  system(("bash " + get_exe_dir() + "/../scripts/exit.sh").c_str());

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

  return 0;
}

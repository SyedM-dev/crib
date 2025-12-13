#include "../include/main.h"
#include "../include/editor.h"
#include "../include/ts.h"
#include "../include/ui.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <sys/ioctl.h>
#include <thread>

std::atomic<bool> running{true};
Queue<KeyEvent> event_queue;
std::vector<Editor *> editors;

uint8_t current_editor = 0;
uint8_t mode = NORMAL;

void background_worker() {
  while (running) {
    ts_collect_spans(editors[current_editor]);
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

void input_listener() {
  while (running) {
    KeyEvent event = read_key();
    if (event.key_type == KEY_NONE)
      continue;
    if (event.key_type == KEY_CHAR && event.len == 1 && *event.c == CTRL('q')) {
      free(event.c);
      running = false;
      return;
    }
    event_queue.push(event);
    std::this_thread::sleep_for(std::chrono::microseconds(100));
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
  for (uint8_t i = 0; i < editors.size(); i++) {
    if (editors[i] == ed)
      return i;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  Coord screen = start_screen();
  const char *filename = (argc > 1) ? argv[1] : "";

  Editor *editor = new_editor(filename, {0, 0}, screen);

  if (!editor) {
    end_screen();
    fprintf(stderr, "Failed to load editor\n");
    return 1;
  }

  editors.push_back(editor);
  current_editor = editors.size() - 1;

  std::thread input_thread(input_listener);
  std::thread work_thread(background_worker);

  while (running) {
    KeyEvent event;
    while (event_queue.pop(event)) {
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
    }

    render_editor(editors[current_editor]);
    render();

    std::this_thread::sleep_for(std::chrono::milliseconds(8));
  }

  if (input_thread.joinable())
    input_thread.join();

  if (work_thread.joinable())
    work_thread.join();

  end_screen();

  free_editor(editor);
  clear_regex_cache();
  return 0;
}

#include "../include/main.h"
#include "../include/editor.h"
#include "../include/ts.h"
#include "../include/ui.h"
#include <atomic>
#include <chrono>
#include <sys/ioctl.h>
#include <thread>

std::atomic<bool> running{true};
Queue<KeyEvent> event_queue;

uint8_t mode = INSERT;

void background_worker(Editor *editor) {
  while (running) {
    ts_collect_spans(editor);
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

void input_listener() {
  while (running) {
    KeyEvent event = read_key();
    if (event.key_type == KEY_NONE)
      continue;
    if (event.key_type == KEY_CHAR && *event.c == CTRL('q'))
      running = false;
    event_queue.push(event);
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}

void handle_editor_event(Editor *editor, KeyEvent event) {
  if (event.key_type == KEY_SPECIAL) {
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
  }
  switch (mode) {
  case NORMAL:
    break;
  case INSERT:
    if (event.key_type == KEY_CHAR) {
      if (event.len == 1) {
        if (event.c[0] == '\t') {
          edit_insert(editor, editor->cursor, (char *)"  ", 1);
          cursor_right(editor, 2);
        } else if (event.c[0] == '\n' || event.c[0] == '\r') {
          edit_insert(editor, editor->cursor, (char *)"\n", 1);
          cursor_right(editor, 1);
        } else if (event.c[0] == 0x7F) {
          edit_erase(editor, editor->cursor, -1);
        } else if (isprint((unsigned char)(event.c[0]))) {
          edit_insert(editor, editor->cursor, event.c, 1);
          cursor_right(editor, 1);
        }
      } else if (event.len > 1) {
        edit_insert(editor, editor->cursor, event.c, event.len);
        cursor_right(editor, 1);
      }
    }
    if (event.key_type == KEY_SPECIAL && event.special_key == KEY_DELETE)
      edit_erase(editor, editor->cursor, 1);
    break;
  }
  ensure_scroll(editor);
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

  std::thread input_thread(input_listener);
  std::thread work_thread(background_worker, editor);

  while (running) {
    KeyEvent event;
    while (event_queue.pop(event))
      handle_editor_event(editor, event);

    render_editor(editor);
    render();

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  input_thread.detach();

  if (work_thread.joinable())
    work_thread.join();

  end_screen();

  free_editor(editor);
  clear_regex_cache();
  return 0;
}

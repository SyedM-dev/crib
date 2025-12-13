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

uint8_t mode = NORMAL;

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
    if (event.key_type == KEY_CHAR && event.len == 1) {
      switch (event.c[0]) {
      case 'a':
      case 'i':
        mode = INSERT;
        break;
      case 's':
      case 'v':
        mode = SELECT;
        editor->selection_active = true;
        editor->selection = editor->cursor;
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
      }
    }
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
        } else if (event.c[0] == 0x1B) {
          mode = NORMAL;
        }
      } else if (event.len > 1) {
        edit_insert(editor, editor->cursor, event.c, event.len);
        cursor_right(editor, 1);
      }
    }
    if (event.key_type == KEY_SPECIAL && event.special_key == KEY_DELETE)
      edit_erase(editor, editor->cursor, 1);
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
      }
    }
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
  if (event.key_type == KEY_CHAR && event.c)
    free(event.c);
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

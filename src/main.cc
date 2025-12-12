#include "../include/editor.h"
#include "../include/ts.h"
#include "../include/ui.h"
#include <atomic>
#include <chrono>
#include <sys/ioctl.h>
#include <thread>

std::atomic<bool> running{true};
Queue<KeyEvent> event_queue;

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
    if (event.key_type == KEY_CHAR && event.c == CTRL('q'))
      running = false;
    event_queue.push(event);
  }
}

void handle_editor_event(Editor *editor, KeyEvent event) {
  if (event.key_type == KEY_SPECIAL && event.special_key == KEY_DOWN)
    cursor_down(editor, 1);
  if (event.key_type == KEY_SPECIAL && event.special_key == KEY_UP)
    cursor_up(editor, 1);
  if (event.key_type == KEY_SPECIAL && event.special_key == KEY_LEFT)
    cursor_left(editor, 1);
  if (event.key_type == KEY_SPECIAL && event.special_key == KEY_RIGHT)
    cursor_right(editor, 1);
  if (event.key_type == KEY_CHAR &&
      ((event.c >= 'a' && event.c <= 'z') ||
       (event.c >= 'A' && event.c <= 'Z') ||
       (event.c >= '0' && event.c <= '9') || event.c == ' ' || event.c == '!' ||
       event.c == '@' || event.c == '#' || event.c == '$' || event.c == '%' ||
       event.c == '^' || event.c == '&' || event.c == '*' || event.c == '(' ||
       event.c == ')' || event.c == '-' || event.c == '_' || event.c == '=' ||
       event.c == '+' || event.c == '[' || event.c == ']' || event.c == '{' ||
       event.c == '}' || event.c == '\\' || event.c == '|' || event.c == ';' ||
       event.c == ':' || event.c == '\'' || event.c == '"' || event.c == ',' ||
       event.c == '.' || event.c == '<' || event.c == '>' || event.c == '/' ||
       event.c == '?' || event.c == '`' || event.c == '~')) {
    edit_insert(editor, editor->cursor, &event.c, 1);
    cursor_right(editor, 1);
  }
  if (event.key_type == KEY_CHAR && event.c == '\t') {
    edit_insert(editor, editor->cursor, (char *)"\t", 1);
    cursor_right(editor, 2);
  }
  if (event.key_type == KEY_CHAR && (event.c == '\n' || event.c == '\r')) {
    edit_insert(editor, editor->cursor, (char *)"\n", 1);
    cursor_right(editor, 1);
  }
  if (event.key_type == KEY_CHAR && event.c == 0x7F)
    edit_erase(editor, editor->cursor, -1);
  if (event.key_type == KEY_SPECIAL && event.special_key == KEY_DELETE)
    edit_erase(editor, editor->cursor, 1);
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

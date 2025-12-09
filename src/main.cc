#include "../include/editor.h"
#include "../include/ts.h"
#include "../include/ui.h"
#include "../libs/tree-sitter/lib/include/tree_sitter/api.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <sys/ioctl.h>
#include <thread>

std::atomic<bool> running{true};
Queue<KeyEvent> event_queue;

std::atomic<uint64_t> render_frames{0};
std::atomic<uint64_t> worker_frames{0};

auto start_time = std::chrono::high_resolution_clock::now();

void background_worker(Editor *editor) {
  while (running) {
    worker_frames++;

    ts_collect_spans(editor);

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

void input_listener() {
  while (running) {
    KeyEvent event = read_key();
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
    std::shared_lock lock_1(editor->knot_mtx);
    uint32_t pos = line_to_byte(editor->root, editor->cursor.row, nullptr) +
                   editor->cursor.col;
    lock_1.unlock();
    std::unique_lock lock_2(editor->knot_mtx);
    editor->root = insert(editor->root, pos, &event.c, 1);
    lock_2.unlock();
    if (editor->tree) {
      TSInputEdit edit = {
          .start_byte = pos,
          .old_end_byte = pos,
          .new_end_byte = pos + 1,
          .start_point = {editor->cursor.row, editor->cursor.col},
          .old_end_point = {editor->cursor.row, editor->cursor.col},
          .new_end_point = {editor->cursor.row, editor->cursor.col + 1},
      };
      editor->edit_queue.push(edit);
    }
    cursor_right(editor, 1);
    apply_edit(editor->spans.spans, pos, 1);
    if (editor->spans.mid_parse)
      editor->spans.edits.push({pos, 1});
  }
  if (event.key_type == KEY_CHAR && event.c == '\t') {
    std::shared_lock lock_1(editor->knot_mtx);
    uint32_t pos = line_to_byte(editor->root, editor->cursor.row, nullptr) +
                   editor->cursor.col;
    lock_1.unlock();
    std::unique_lock lock_2(editor->knot_mtx);
    editor->root = insert(editor->root, pos, (char *)"  ", 2);
    lock_2.unlock();
    if (editor->tree) {
      TSInputEdit edit = {
          .start_byte = pos,
          .old_end_byte = pos,
          .new_end_byte = pos + 2,
          .start_point = {editor->cursor.row, editor->cursor.col},
          .old_end_point = {editor->cursor.row, editor->cursor.col},
          .new_end_point = {editor->cursor.row, editor->cursor.col + 2},
      };
      editor->edit_queue.push(edit);
    }
    cursor_right(editor, 2);
    std::unique_lock lock_3(editor->spans.mtx);
    apply_edit(editor->spans.spans, pos, 2);
    if (editor->spans.mid_parse)
      editor->spans.edits.push({pos, 2});
  }
  if (event.key_type == KEY_CHAR && (event.c == '\n' || event.c == '\r')) {
    std::shared_lock lock_1(editor->knot_mtx);
    uint32_t pos = line_to_byte(editor->root, editor->cursor.row, nullptr) +
                   editor->cursor.col;
    lock_1.unlock();
    std::unique_lock lock_2(editor->knot_mtx);
    editor->root = insert(editor->root, pos, (char *)"\n", 1);
    editor->folded.resize(editor->root->line_count + 2);
    lock_2.unlock();
    if (editor->tree) {
      TSInputEdit edit = {
          .start_byte = pos,
          .old_end_byte = pos,
          .new_end_byte = pos + 1,
          .start_point = {editor->cursor.row, editor->cursor.col},
          .old_end_point = {editor->cursor.row, editor->cursor.col},
          .new_end_point = {editor->cursor.row + 1, 0},
      };
      editor->edit_queue.push(edit);
    }
    cursor_right(editor, 1);
    std::unique_lock lock_3(editor->spans.mtx);
    apply_edit(editor->spans.spans, pos + 1, 1);
    if (editor->spans.mid_parse)
      editor->spans.edits.push({pos + 1, 1});
  }
  if (event.key_type == KEY_CHAR && event.c == 0x7F) {
    std::shared_lock lock_1(editor->knot_mtx);
    uint32_t pos = line_to_byte(editor->root, editor->cursor.row, nullptr) +
                   editor->cursor.col;
    TSPoint old_point = {editor->cursor.row, editor->cursor.col};
    cursor_left(editor, 1);
    uint32_t start = line_to_byte(editor->root, editor->cursor.row, nullptr) +
                     editor->cursor.col;
    lock_1.unlock();
    std::unique_lock lock_2(editor->knot_mtx);
    editor->root = erase(editor->root, start, pos - start);
    lock_2.unlock();
    if (editor->tree) {
      TSInputEdit edit = {
          .start_byte = start,
          .old_end_byte = pos,
          .new_end_byte = start,
          .start_point = {editor->cursor.row, editor->cursor.col},
          .old_end_point = old_point,
          .new_end_point = {editor->cursor.row, editor->cursor.col},
      };
      editor->edit_queue.push(edit);
    }
    std::unique_lock lock_3(editor->spans.mtx);
    apply_edit(editor->spans.spans, start, start - pos);
    if (editor->spans.mid_parse)
      editor->spans.edits.push({start, start - pos});
  }
  ensure_scroll(editor);
}

int main(int argc, char *argv[]) {
  Coord screen = start_screen();
  const char *filename = (argc > 1) ? argv[1] : "ts.cpp";

  Editor *editor = new_editor(filename, {0, 0}, {screen.row, screen.col});
  if (!editor) {
    end_screen();
    fprintf(stderr, "Failed to load editor\n");
    return 1;
  }

  start_time = std::chrono::high_resolution_clock::now();

  std::thread input_thread(input_listener);
  std::thread work_thread(background_worker, editor);

  while (running) {
    render_frames++;

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

  auto end_time = std::chrono::high_resolution_clock::now();
  double seconds = std::chrono::duration<double>(end_time - start_time).count();

  double render_fps = render_frames / seconds;
  double worker_fps = worker_frames / seconds;

  end_screen();

  std::cout << "\n======= Performance Summary =======\n";
  std::cout << "Runtime: " << seconds << "s\n";
  std::cout << "Render loop FPS: " << render_fps << "Hz\n";
  std::cout << "Worker loop FPS: " << worker_fps << "Hz\n";
  std::cout << "===================================\n";

  free_editor(editor);
  return 0;
}

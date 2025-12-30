#include "editor/editor.h"
#include "editor/folds.h"
#include "main.h"

void move_line_up(Editor *editor) {
  if (!editor || !editor->root || editor->cursor.row == 0)
    return;
  if (mode == NORMAL || mode == INSERT) {
    uint32_t line_len, line_cluster_len;
    std::shared_lock lock(editor->knot_mtx);
    LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
    char *line = next_line(it, &line_len);
    if (!line) {
      lock.unlock();
      return;
    }
    if (line_len > 0 && line[line_len - 1] == '\n')
      line_len--;
    line_cluster_len = count_clusters(line, line_len, 0, line_len);
    uint32_t target_row = prev_unfolded_row(editor, editor->cursor.row - 1);
    uint32_t up_by = editor->cursor.row - target_row;
    if (up_by > 1)
      up_by--;
    lock.unlock();
    Coord cursor = editor->cursor;
    edit_erase(editor, {cursor.row, 0}, line_cluster_len);
    edit_erase(editor, {cursor.row, 0}, -1);
    edit_insert(editor, {cursor.row - up_by, 0}, (char *)"\n", 1);
    edit_insert(editor, {cursor.row - up_by, 0}, line, line_len);
    free(it->buffer);
    free(it);
    editor->cursor = {cursor.row - up_by, cursor.col};
  } else if (mode == SELECT) {
    uint32_t start_row = MIN(editor->cursor.row, editor->selection.row);
    uint32_t end_row = MAX(editor->cursor.row, editor->selection.row);
    uint32_t start_byte = line_to_byte(editor->root, start_row, nullptr);
    uint32_t end_byte = line_to_byte(editor->root, end_row + 1, nullptr);
    char *selected_text = read(editor->root, start_byte, end_byte - start_byte);
    if (!selected_text)
      return;
    uint32_t selected_len = count_clusters(selected_text, end_byte - start_byte,
                                           0, end_byte - start_byte);
    Coord cursor = editor->cursor;
    Coord selection = editor->selection;
    edit_erase(editor, {start_row, 0}, selected_len);
    edit_insert(editor, {start_row - 1, 0}, selected_text,
                end_byte - start_byte);
    free(selected_text);
    editor->cursor = {cursor.row - 1, cursor.col};
    editor->selection = {selection.row - 1, selection.col};
  }
}

void move_line_down(Editor *editor) {
  if (!editor || !editor->root)
    return;
  if (mode == NORMAL || mode == INSERT) {
    if (editor->cursor.row >= editor->root->line_count - 1)
      return;
    uint32_t line_len, line_cluster_len;
    std::shared_lock lock(editor->knot_mtx);
    LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
    char *line = next_line(it, &line_len);
    if (!line) {
      lock.unlock();
      return;
    }
    if (line_len && line[line_len - 1] == '\n')
      line_len--;
    line_cluster_len = count_clusters(line, line_len, 0, line_len);
    uint32_t target_row = next_unfolded_row(editor, editor->cursor.row + 1);
    if (target_row >= editor->root->line_count) {
      free(line);
      lock.unlock();
      return;
    }
    uint32_t down_by = target_row - editor->cursor.row;
    if (down_by > 1)
      down_by--;
    uint32_t ln;
    line_to_byte(editor->root, editor->cursor.row + down_by - 1, &ln);
    lock.unlock();
    Coord cursor = editor->cursor;
    edit_erase(editor, {cursor.row, 0}, line_cluster_len);
    edit_erase(editor, {cursor.row, 0}, -1);
    edit_insert(editor, {cursor.row + down_by, 0}, (char *)"\n", 1);
    edit_insert(editor, {cursor.row + down_by, 0}, line, line_len);
    free(it->buffer);
    free(it);
    editor->cursor = {cursor.row + down_by, cursor.col};
  } else if (mode == SELECT) {
    if (editor->cursor.row >= editor->root->line_count - 1 ||
        editor->selection.row >= editor->root->line_count - 1)
      return;
    std::shared_lock lock(editor->knot_mtx);
    uint32_t start_row = MIN(editor->cursor.row, editor->selection.row);
    uint32_t end_row = MAX(editor->cursor.row, editor->selection.row);
    uint32_t target_row = next_unfolded_row(editor, end_row + 1);
    if (target_row >= editor->root->line_count)
      return;
    uint32_t down_by = target_row - end_row;
    if (down_by > 1)
      down_by--;
    uint32_t start_byte = line_to_byte(editor->root, start_row, nullptr);
    uint32_t end_byte = line_to_byte(editor->root, end_row + 1, nullptr);
    char *selected_text = read(editor->root, start_byte, end_byte - start_byte);
    lock.unlock();
    if (!selected_text)
      return;
    uint32_t selected_len = count_clusters(selected_text, end_byte - start_byte,
                                           0, end_byte - start_byte);
    Coord cursor = editor->cursor;
    Coord selection = editor->selection;
    edit_erase(editor, {start_row, 0}, selected_len);
    edit_insert(editor, {start_row + down_by, 0}, selected_text,
                end_byte - start_byte);
    free(selected_text);
    editor->cursor = {cursor.row + down_by, cursor.col};
    editor->selection = {selection.row + down_by, selection.col};
  }
}

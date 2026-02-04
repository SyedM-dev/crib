#include "editor/editor.h"
#include "main.h"

void Editor::move_line_up() {
  if (!this->root || this->cursor.row == 0)
    return;
  if (mode == NORMAL || mode == INSERT) {
    uint32_t line_len, line_cluster_len;
    LineIterator *it = begin_l_iter(this->root, this->cursor.row);
    char *line = next_line(it, &line_len);
    if (!line)
      return;
    if (line_len > 0 && line[line_len - 1] == '\n')
      line_len--;
    line_cluster_len = count_clusters(line, line_len, 0, line_len);
    uint32_t target_row = this->cursor.row - 1;
    uint32_t up_by = this->cursor.row - target_row;
    if (up_by > 1)
      up_by--;
    Coord cursor = this->cursor;
    edit_erase({cursor.row, 0}, line_cluster_len);
    edit_erase({cursor.row, 0}, -1);
    edit_insert({cursor.row - up_by, 0}, (char *)"\n", 1);
    edit_insert({cursor.row - up_by, 0}, line, line_len);
    free(it->buffer);
    free(it);
    this->cursor = {cursor.row - up_by, cursor.col};
  } else if (mode == SELECT) {
    uint32_t start_row = MIN(this->cursor.row, this->selection.row);
    uint32_t end_row = MAX(this->cursor.row, this->selection.row);
    uint32_t start_byte = line_to_byte(this->root, start_row, nullptr);
    uint32_t end_byte = line_to_byte(this->root, end_row + 1, nullptr);
    char *selected_text = read(this->root, start_byte, end_byte - start_byte);
    if (!selected_text)
      return;
    uint32_t selected_len = count_clusters(selected_text, end_byte - start_byte,
                                           0, end_byte - start_byte);
    Coord cursor = this->cursor;
    Coord selection = this->selection;
    edit_erase({start_row, 0}, selected_len);
    edit_insert({start_row - 1, 0}, selected_text, end_byte - start_byte);
    free(selected_text);
    this->cursor = {cursor.row - 1, cursor.col};
    this->selection = {selection.row - 1, selection.col};
  }
}

void Editor::move_line_down() {
  if (!this->root)
    return;
  if (mode == NORMAL || mode == INSERT) {
    if (this->cursor.row >= this->root->line_count - 1)
      return;
    uint32_t line_len, line_cluster_len;
    LineIterator *it = begin_l_iter(this->root, this->cursor.row);
    char *line = next_line(it, &line_len);
    if (!line)
      return;
    if (line_len && line[line_len - 1] == '\n')
      line_len--;
    line_cluster_len = count_clusters(line, line_len, 0, line_len);
    uint32_t target_row = this->cursor.row + 1;
    if (target_row >= this->root->line_count) {
      free(it->buffer);
      free(it);
      return;
    }
    uint32_t down_by = target_row - this->cursor.row;
    if (down_by > 1)
      down_by--;
    uint32_t ln;
    line_to_byte(this->root, this->cursor.row + down_by - 1, &ln);
    Coord cursor = this->cursor;
    edit_erase({cursor.row, 0}, line_cluster_len);
    edit_erase({cursor.row, 0}, -1);
    edit_insert({cursor.row + down_by, 0}, (char *)"\n", 1);
    edit_insert({cursor.row + down_by, 0}, line, line_len);
    free(it->buffer);
    free(it);
    this->cursor = {cursor.row + down_by, cursor.col};
  } else if (mode == SELECT) {
    if (this->cursor.row >= this->root->line_count - 1 ||
        this->selection.row >= this->root->line_count - 1)
      return;
    uint32_t start_row = MIN(this->cursor.row, this->selection.row);
    uint32_t end_row = MAX(this->cursor.row, this->selection.row);
    uint32_t target_row = end_row + 1;
    if (target_row >= this->root->line_count)
      return;
    uint32_t down_by = target_row - end_row;
    if (down_by > 1)
      down_by--;
    uint32_t start_byte = line_to_byte(this->root, start_row, nullptr);
    uint32_t end_byte = line_to_byte(this->root, end_row + 1, nullptr);
    char *selected_text = read(this->root, start_byte, end_byte - start_byte);
    if (!selected_text)
      return;
    uint32_t selected_len = count_clusters(selected_text, end_byte - start_byte,
                                           0, end_byte - start_byte);
    Coord cursor = this->cursor;
    Coord selection = this->selection;
    edit_erase({start_row, 0}, selected_len);
    edit_insert({start_row + down_by, 0}, selected_text, end_byte - start_byte);
    free(selected_text);
    this->cursor = {cursor.row + down_by, cursor.col};
    this->selection = {selection.row + down_by, selection.col};
  }
}

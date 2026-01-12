#include "editor/editor.h"
#include "editor/folds.h"
#include "lsp/lsp.h"
#include "utils/utils.h"

void edit_erase(Editor *editor, Coord pos, int64_t len) {
  if (len == 0)
    return;
  if (len < 0) {
    std::shared_lock lock_1(editor->knot_mtx);
    uint32_t cursor_original =
        line_to_byte(editor->root, editor->cursor.row, nullptr) +
        editor->cursor.col;
    TSPoint old_point = {pos.row, pos.col};
    uint32_t byte_pos = line_to_byte(editor->root, pos.row, nullptr) + pos.col;
    Coord point = move_left_pure(editor, pos, -len);
    json lsp_range;
    bool do_lsp = (editor->lsp != nullptr);
    if (do_lsp) {
      LineIterator *it = begin_l_iter(editor->root, point.row);
      char *line = next_line(it, nullptr);
      int utf16_start = 0;
      if (line)
        utf16_start = utf8_byte_offset_to_utf16(line, point.col);
      free(it->buffer);
      free(it);
      it = begin_l_iter(editor->root, pos.row);
      line = next_line(it, nullptr);
      int utf16_end = 0;
      if (line)
        utf16_end = utf8_byte_offset_to_utf16(line, pos.col);
      free(it->buffer);
      free(it);
      lsp_range = {{"start", {{"line", point.row}, {"character", utf16_start}}},
                   {"end", {{"line", pos.row}, {"character", utf16_end}}}};
    }
    uint32_t start = line_to_byte(editor->root, point.row, nullptr) + point.col;
    if (cursor_original > start && cursor_original <= byte_pos) {
      editor->cursor = point;
      editor->cursor_preffered = UINT32_MAX;
    } else if (cursor_original > byte_pos) {
      uint32_t cursor_new = cursor_original - (byte_pos - start);
      uint32_t new_col;
      uint32_t new_row = byte_to_line(editor->root, cursor_new, &new_col);
      editor->cursor = {new_row, new_col};
      editor->cursor_preffered = UINT32_MAX;
    }
    lock_1.unlock();
    uint32_t start_row = point.row;
    uint32_t end_row = pos.row;
    apply_line_deletion(editor, start_row + 1, end_row);
    apply_hook_deletion(editor, start_row + 1, end_row);
    std::unique_lock lock_2(editor->knot_mtx);
    editor->root = erase(editor->root, start, byte_pos - start);
    if (editor->ts.tree) {
      TSInputEdit edit = {
          .start_byte = start,
          .old_end_byte = byte_pos,
          .new_end_byte = start,
          .start_point = {point.row, point.col},
          .old_end_point = old_point,
          .new_end_point = {point.row, point.col},
      };
      editor->edit_queue.push(edit);
    }
    std::unique_lock lock_3(editor->spans.mtx);
    apply_edit(editor->spans.spans, start, start - byte_pos);
    if (editor->spans.mid_parse)
      editor->spans.edits.push({start, start - byte_pos});
    lock_3.unlock();
    lock_2.unlock();
    std::unique_lock lock_4(editor->hex_color_spans.mtx);
    apply_edit(editor->hex_color_spans.spans, byte_pos, start - byte_pos);
    lock_4.unlock();
    if (do_lsp) {
      if (editor->lsp->incremental_sync) {
        json message = {
            {"jsonrpc", "2.0"},
            {"method", "textDocument/didChange"},
            {"params",
             {{"textDocument",
               {{"uri", editor->uri}, {"version", ++editor->lsp_version}}},
              {"contentChanges",
               json::array({{{"range", lsp_range}, {"text", ""}}})}}}};
        lsp_send(editor->lsp, message, nullptr);
      } else {
        char *buf = read(editor->root, 0, editor->root->char_count);
        std::string text(buf);
        free(buf);
        json message = {
            {"jsonrpc", "2.0"},
            {"method", "textDocument/didChange"},
            {"params",
             {{"textDocument",
               {{"uri", editor->uri}, {"version", ++editor->lsp_version}}},
              {"contentChanges", json::array({{{"text", text}}})}}}};
        lsp_send(editor->lsp, message, nullptr);
      }
    }
  } else {
    std::shared_lock lock_1(editor->knot_mtx);
    uint32_t cursor_original =
        line_to_byte(editor->root, editor->cursor.row, nullptr) +
        editor->cursor.col;
    TSPoint old_point = {pos.row, pos.col};
    uint32_t byte_pos = line_to_byte(editor->root, pos.row, nullptr) + pos.col;
    Coord point = move_right_pure(editor, pos, len);
    json lsp_range;
    bool do_lsp = (editor->lsp != nullptr);
    if (do_lsp) {
      LineIterator *it = begin_l_iter(editor->root, pos.row);
      char *line = next_line(it, nullptr);
      int utf16_start = 0;
      if (line)
        utf16_start = utf8_byte_offset_to_utf16(line, pos.col);
      free(it->buffer);
      free(it);
      it = begin_l_iter(editor->root, point.row);
      line = next_line(it, nullptr);
      int utf16_end = 0;
      if (line)
        utf16_end = utf8_byte_offset_to_utf16(line, point.col);
      free(it->buffer);
      free(it);
      lsp_range = {{"start", {{"line", pos.row}, {"character", utf16_start}}},
                   {"end", {{"line", point.row}, {"character", utf16_end}}}};
    }
    uint32_t end = line_to_byte(editor->root, point.row, nullptr) + point.col;
    if (cursor_original > byte_pos && cursor_original <= end) {
      editor->cursor = pos;
      editor->cursor_preffered = UINT32_MAX;
    } else if (cursor_original > end) {
      uint32_t cursor_new = cursor_original - (end - byte_pos);
      uint32_t new_col;
      uint32_t new_row = byte_to_line(editor->root, cursor_new, &new_col);
      editor->cursor = {new_row, new_col};
      editor->cursor_preffered = UINT32_MAX;
    }
    lock_1.unlock();
    uint32_t start_row = pos.row;
    uint32_t end_row = point.row;
    apply_line_deletion(editor, start_row + 1, end_row);
    apply_hook_deletion(editor, start_row + 1, end_row);
    std::unique_lock lock_2(editor->knot_mtx);
    editor->root = erase(editor->root, byte_pos, end - byte_pos);
    if (editor->ts.tree) {
      TSInputEdit edit = {
          .start_byte = byte_pos,
          .old_end_byte = end,
          .new_end_byte = byte_pos,
          .start_point = old_point,
          .old_end_point = {point.row, point.col},
          .new_end_point = old_point,
      };
      editor->edit_queue.push(edit);
    }
    std::unique_lock lock_3(editor->spans.mtx);
    apply_edit(editor->spans.spans, byte_pos, byte_pos - end);
    if (editor->spans.mid_parse)
      editor->spans.edits.push({byte_pos, byte_pos - end});
    lock_3.unlock();
    lock_2.unlock();
    std::unique_lock lock_4(editor->hex_color_spans.mtx);
    apply_edit(editor->hex_color_spans.spans, byte_pos, byte_pos - end);
    lock_4.unlock();
    if (do_lsp) {
      if (editor->lsp->incremental_sync) {
        json message = {
            {"jsonrpc", "2.0"},
            {"method", "textDocument/didChange"},
            {"params",
             {{"textDocument",
               {{"uri", editor->uri}, {"version", ++editor->lsp_version}}},
              {"contentChanges",
               json::array({{{"range", lsp_range}, {"text", ""}}})}}}};
        lsp_send(editor->lsp, message, nullptr);
      } else {
        char *buf = read(editor->root, 0, editor->root->char_count);
        std::string text(buf);
        free(buf);
        json message = {
            {"jsonrpc", "2.0"},
            {"method", "textDocument/didChange"},
            {"params",
             {{"textDocument",
               {{"uri", editor->uri}, {"version", ++editor->lsp_version}}},
              {"contentChanges", json::array({{{"text", text}}})}}}};
        lsp_send(editor->lsp, message, nullptr);
      }
    }
  }
}

void edit_insert(Editor *editor, Coord pos, char *data, uint32_t len) {
  std::shared_lock lock_1(editor->knot_mtx);
  uint32_t cursor_original =
      line_to_byte(editor->root, editor->cursor.row, nullptr) +
      editor->cursor.col;
  uint32_t byte_pos = line_to_byte(editor->root, pos.row, nullptr) + pos.col;
  TSPoint start_point = {pos.row, pos.col};
  if (cursor_original > byte_pos) {
    uint32_t cursor_new = cursor_original + len;
    uint32_t new_col;
    uint32_t new_row = byte_to_line(editor->root, cursor_new, &new_col);
    editor->cursor = {new_row, new_col};
  }
  lock_1.unlock();
  std::unique_lock lock_2(editor->knot_mtx);
  editor->root = insert(editor->root, byte_pos, data, len);
  uint32_t cols = 0;
  uint32_t rows = 0;
  for (uint32_t i = 0; i < len; i++) {
    if (data[i] == '\n') {
      rows++;
      cols = 0;
    } else {
      cols++;
    }
  }
  apply_line_insertion(editor, pos.row, rows);
  apply_hook_insertion(editor, pos.row, rows);
  if (editor->ts.tree) {
    TSInputEdit edit = {
        .start_byte = byte_pos,
        .old_end_byte = byte_pos,
        .new_end_byte = byte_pos + len,
        .start_point = start_point,
        .old_end_point = start_point,
        .new_end_point = {start_point.row + rows,
                          (rows == 0) ? (start_point.column + cols) : cols},
    };
    editor->edit_queue.push(edit);
  }
  std::unique_lock lock_3(editor->spans.mtx);
  apply_edit(editor->spans.spans, byte_pos, len);
  if (editor->spans.mid_parse)
    editor->spans.edits.push({byte_pos, len});
  lock_3.unlock();
  lock_2.unlock();
  std::unique_lock lock_4(editor->hex_color_spans.mtx);
  apply_edit(editor->hex_color_spans.spans, byte_pos, len);
  lock_4.unlock();
  if (editor->lsp) {
    if (editor->lsp->incremental_sync) {
      lock_1.lock();
      LineIterator *it = begin_l_iter(editor->root, pos.row);
      char *line = next_line(it, nullptr);
      int utf16_col = 0;
      if (line)
        utf16_col = utf8_byte_offset_to_utf16(line, pos.col);
      free(it->buffer);
      free(it);
      lock_1.unlock();
      json message = {
          {"jsonrpc", "2.0"},
          {"method", "textDocument/didChange"},
          {"params",
           {{"textDocument",
             {{"uri", editor->uri}, {"version", ++editor->lsp_version}}},
            {"contentChanges",
             json::array(
                 {{{"range",
                    {{"start", {{"line", pos.row}, {"character", utf16_col}}},
                     {"end", {{"line", pos.row}, {"character", utf16_col}}}}},
                   {"text", std::string(data, len)}}})}}}};
      lsp_send(editor->lsp, message, nullptr);
    } else {
      char *buf = read(editor->root, 0, editor->root->char_count);
      std::string text(buf);
      free(buf);
      json message = {
          {"jsonrpc", "2.0"},
          {"method", "textDocument/didChange"},
          {"params",
           {{"textDocument",
             {{"uri", editor->uri}, {"version", ++editor->lsp_version}}},
            {"contentChanges", json::array({{{"text", text}}})}}}};
      lsp_send(editor->lsp, message, nullptr);
    }
  }
}

void edit_replace(Editor *editor, Coord start, Coord end, const char *text,
                  uint32_t len) {
  std::shared_lock lock(editor->knot_mtx);
  uint32_t start_byte =
      line_to_byte(editor->root, start.row, nullptr) + start.col;
  uint32_t end_byte = line_to_byte(editor->root, end.row, nullptr) + end.col;
  char *buf = read(editor->root, start_byte, end_byte - start_byte);
  if (!buf)
    return;
  lock.unlock();
  uint32_t erase_len =
      count_clusters(buf, end_byte - start_byte, 0, end_byte - start_byte);
  free(buf);
  if (erase_len != 0)
    edit_erase(editor, start, erase_len);
  if (len > 0)
    edit_insert(editor, start, const_cast<char *>(text), len);
}

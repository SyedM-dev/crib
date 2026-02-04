#include "editor/editor.h"
#include "lsp/lsp.h"
#include "utils/utils.h"

void Editor::edit_erase(Coord pos, int64_t len) {
  if (len == 0)
    return;
  if (len < 0) {
    uint32_t cursor_original =
        line_to_byte(this->root, this->cursor.row, nullptr) + this->cursor.col;
    uint32_t byte_pos = line_to_byte(this->root, pos.row, nullptr) + pos.col;
    Coord point = this->move_left(pos, -len);
    json lsp_range;
    bool do_lsp = (this->lsp != nullptr);
    if (do_lsp) {
      LineIterator *it = begin_l_iter(this->root, point.row);
      uint32_t len;
      char *line = next_line(it, &len);
      int utf16_start = 0;
      if (line)
        utf16_start = utf8_offset_to_utf16(line, len, point.col);
      free(it->buffer);
      free(it);
      it = begin_l_iter(this->root, pos.row);
      line = next_line(it, &len);
      int utf16_end = 0;
      if (line)
        utf16_end = utf8_offset_to_utf16(line, len, pos.col);
      free(it->buffer);
      free(it);
      lsp_range = {{"start", {{"line", point.row}, {"character", utf16_start}}},
                   {"end", {{"line", pos.row}, {"character", utf16_end}}}};
    }
    uint32_t start = line_to_byte(this->root, point.row, nullptr) + point.col;
    if (cursor_original > start && cursor_original <= byte_pos) {
      this->cursor = point;
      this->cursor_preffered = UINT32_MAX;
    } else if (cursor_original > byte_pos) {
      uint32_t cursor_new = cursor_original - (byte_pos - start);
      uint32_t new_col;
      uint32_t new_row = byte_to_line(this->root, cursor_new, &new_col);
      this->cursor = {new_row, new_col};
      this->cursor_preffered = UINT32_MAX;
    }
    uint32_t start_row = point.row;
    uint32_t end_row = pos.row;
    this->apply_hook_deletion(start_row + 1, end_row);
    this->root = erase(this->root, start, byte_pos - start);
    if (this->parser)
      this->parser->edit(start_row, end_row - start_row, 0);
    if (do_lsp) {
      auto lsp = this->lsp.load();
      if (lsp->incremental_sync) {
        auto message = std::make_unique<LSPMessage>();
        message->message = {
            {"method", "textDocument/didChange"},
            {"params",
             {{"textDocument",
               {{"uri", this->uri}, {"version", ++this->lsp_version}}},
              {"contentChanges",
               json::array({{{"range", lsp_range}, {"text", ""}}})}}}};
        lsp->send(std::move(message));
      } else {
        char *buf = read(this->root, 0, this->root->char_count);
        std::string text(buf);
        free(buf);
        auto message = std::make_unique<LSPMessage>();
        message->message = {
            {"method", "textDocument/didChange"},
            {"params",
             {{"textDocument",
               {{"uri", this->uri}, {"version", ++this->lsp_version}}},
              {"contentChanges", json::array({{{"text", text}}})}}}};
        lsp->send(std::move(message));
      }
    }
  } else {
    uint32_t cursor_original =
        line_to_byte(this->root, this->cursor.row, nullptr) + this->cursor.col;
    uint32_t byte_pos = line_to_byte(this->root, pos.row, nullptr) + pos.col;
    Coord point = this->move_right(pos, len);
    json lsp_range;
    bool do_lsp = (this->lsp != nullptr);
    if (do_lsp) {
      LineIterator *it = begin_l_iter(this->root, pos.row);
      uint32_t line_len;
      char *line = next_line(it, &line_len);
      int utf16_start = 0;
      if (line)
        utf16_start = utf8_offset_to_utf16(line, line_len, pos.col);
      free(it->buffer);
      free(it);
      it = begin_l_iter(this->root, point.row);
      line = next_line(it, &line_len);
      int utf16_end = 0;
      if (line)
        utf16_end = utf8_offset_to_utf16(line, line_len, point.col);
      free(it->buffer);
      free(it);
      lsp_range = {{"start", {{"line", pos.row}, {"character", utf16_start}}},
                   {"end", {{"line", point.row}, {"character", utf16_end}}}};
    }
    uint32_t end = line_to_byte(this->root, point.row, nullptr) + point.col;
    if (cursor_original > byte_pos && cursor_original <= end) {
      this->cursor = pos;
      this->cursor_preffered = UINT32_MAX;
    } else if (cursor_original > end) {
      uint32_t cursor_new = cursor_original - (end - byte_pos);
      uint32_t new_col;
      uint32_t new_row = byte_to_line(this->root, cursor_new, &new_col);
      this->cursor = {new_row, new_col};
      this->cursor_preffered = UINT32_MAX;
    }
    uint32_t start_row = pos.row;
    uint32_t end_row = point.row;
    this->apply_hook_deletion(start_row + 1, end_row);
    this->root = erase(this->root, byte_pos, end - byte_pos);
    if (this->parser)
      this->parser->edit(start_row, end_row - start_row, 0);
    if (do_lsp) {
      auto lsp = this->lsp.load();
      if (lsp->incremental_sync) {
        auto message = std::make_unique<LSPMessage>();
        message->message = {
            {"method", "textDocument/didChange"},
            {"params",
             {{"textDocument",
               {{"uri", this->uri}, {"version", ++this->lsp_version}}},
              {"contentChanges",
               json::array({{{"range", lsp_range}, {"text", ""}}})}}}};
        lsp->send(std::move(message));
      } else {
        char *buf = read(this->root, 0, this->root->char_count);
        std::string text(buf);
        free(buf);
        auto message = std::make_unique<LSPMessage>();
        message->message = {
            {"method", "textDocument/didChange"},
            {"params",
             {{"textDocument",
               {{"uri", this->uri}, {"version", ++this->lsp_version}}},
              {"contentChanges", json::array({{{"text", text}}})}}}};
        lsp->send(std::move(message));
      }
    }
  }
}

void Editor::edit_insert(Coord pos, char *data, uint32_t len) {
  uint32_t cursor_original =
      line_to_byte(this->root, this->cursor.row, nullptr) + this->cursor.col;
  uint32_t byte_pos = line_to_byte(this->root, pos.row, nullptr) + pos.col;
  if (cursor_original > byte_pos) {
    uint32_t cursor_new = cursor_original + len;
    uint32_t new_col;
    uint32_t new_row = byte_to_line(this->root, cursor_new, &new_col);
    this->cursor = {new_row, new_col};
  }
  LineIterator *it = begin_l_iter(this->root, pos.row);
  uint32_t line_len;
  char *line = next_line(it, &line_len);
  int utf16_col = 0;
  if (line)
    utf16_col = utf8_offset_to_utf16(line, line_len, pos.col);
  free(it->buffer);
  free(it);
  this->root = insert(this->root, byte_pos, data, len);
  uint32_t rows = 0;
  for (uint32_t i = 0; i < len; i++)
    if (data[i] == '\n')
      rows++;
  this->apply_hook_insertion(pos.row, rows);
  if (this->parser)
    this->parser->edit(pos.row, 0, rows);
  auto lsp = this->lsp.load();
  if (lsp) {
    if (lsp->incremental_sync) {
      auto message = std::make_unique<LSPMessage>();
      message->message = {
          {"method", "textDocument/didChange"},
          {"params",
           {{"textDocument",
             {{"uri", this->uri}, {"version", ++this->lsp_version}}},
            {"contentChanges",
             json::array(
                 {{{"range",
                    {{"start", {{"line", pos.row}, {"character", utf16_col}}},
                     {"end", {{"line", pos.row}, {"character", utf16_col}}}}},
                   {"text", std::string(data, len)}}})}}}};
      lsp->send(std::move(message));
    } else {
      char *buf = read(this->root, 0, this->root->char_count);
      std::string text(buf);
      free(buf);
      auto message = std::make_unique<LSPMessage>();
      message->message = {
          {"method", "textDocument/didChange"},
          {"params",
           {{"textDocument",
             {{"uri", this->uri}, {"version", ++this->lsp_version}}},
            {"contentChanges", json::array({{{"text", text}}})}}}};
      lsp->send(std::move(message));
    }
  }
}

void Editor::edit_replace(Coord start, Coord end, const char *text,
                          uint32_t len) {
  uint32_t start_byte =
      line_to_byte(this->root, start.row, nullptr) + start.col;
  uint32_t end_byte = line_to_byte(this->root, end.row, nullptr) + end.col;
  LineIterator *it = begin_l_iter(this->root, start.row);
  uint32_t line_len;
  char *line = next_line(it, &line_len);
  int utf16_start = 0;
  if (line)
    utf16_start = utf8_offset_to_utf16(line, line_len, start.col);
  free(it->buffer);
  free(it);
  it = begin_l_iter(this->root, end.row);
  line = next_line(it, &line_len);
  int utf16_end = 0;
  if (line)
    utf16_end = utf8_offset_to_utf16(line, line_len, end.col);
  free(it->buffer);
  free(it);
  if (start_byte != end_byte)
    this->root = erase(this->root, start_byte, end_byte - start_byte);
  if (len > 0)
    this->root = insert(this->root, start_byte, (char *)text, len);
  uint32_t rows = 0;
  for (uint32_t i = 0; i < len; i++)
    if (text[i] == '\n')
      rows++;
  if (rows > 0)
    rows--;
  if (this->parser)
    this->parser->edit(start.row, end.row - start.row, rows);
  auto lsp = this->lsp.load();
  if (lsp) {
    if (lsp->incremental_sync) {
      auto message = std::make_unique<LSPMessage>();
      message->message = {
          {"method", "textDocument/didChange"},
          {"params",
           {{"textDocument",
             {{"uri", this->uri}, {"version", ++this->lsp_version}}},
            {"contentChanges",
             json::array(
                 {{{"range",
                    {{"start",
                      {{"line", start.row}, {"character", utf16_start}}},
                     {"end", {{"line", end.row}, {"character", utf16_end}}}}},
                   {"text", std::string(text, len)}}})}}}};
      lsp->send(std::move(message));
    } else {
      char *buf = read(this->root, 0, this->root->char_count);
      std::string full_text(buf);
      free(buf);
      auto message = std::make_unique<LSPMessage>();
      message->message = {
          {"method", "textDocument/didChange"},
          {"params",
           {{"textDocument",
             {{"uri", this->uri}, {"version", ++this->lsp_version}}},
            {"contentChanges", json::array({{{"text", full_text}}})}}}};
      lsp->send(std::move(message));
    }
  }
}

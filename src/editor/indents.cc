#include "editor/editor.h"
#include "io/knot.h"
#include "lsp/lsp.h"

void IndentationEngine::compute_indent(Editor *n_editor) {
  editor = n_editor;
  uint32_t line_idx = 0;
  LineIterator *it = begin_l_iter(editor->root, 0);
  if (!it)
    return;
  auto is_set = kLangtoIndent.find(editor->lang.name);
  indent = is_set != kLangtoIndent.end() ? is_set->second : 0;
  while (indent == 0 && line_idx < 64) {
    uint32_t len;
    char *line = next_line(it, &len);
    if (!line)
      break;
    line_idx++;
    if (len > 0 && line[len - 1] == '\n')
      --len;
    if (len == 0)
      continue;
    if (line[0] == '\t') {
      indent = 1;
    } else if (line[0] == ' ') {
      for (uint32_t i = 0; i < len; i++) {
        if (line[i] == ' ') {
          indent += 1;
        } else {
          if (indent == 1)
            indent = 0;
          break;
        }
      }
    }
  }
  if (indent == 0)
    indent = 2;
  free(it->buffer);
  free(it);
}

uint32_t IndentationEngine::indent_real(char *line, uint32_t len) {
  uint32_t spaces = 0;
  uint32_t tabs = 0;
  for (uint32_t i = 0; i < len; i++) {
    if (line[i] == ' ') {
      spaces += 1;
    } else if (line[i] == '\t') {
      tabs += 1;
    } else {
      break;
    }
  }
  return tabs ? tabs : spaces / indent;
}

uint32_t IndentationEngine::indent_expected(uint32_t row) {
  std::shared_lock lock(editor->knot_mtx);
  uint32_t line_idx = row;
  if (row == 0)
    return 0;
  LineIterator *it = begin_l_iter(editor->root, row - 1);
  if (!it)
    return 0;
  next_line(it, nullptr);
  uint32_t c_indent = 0;
  while (line_idx--) {
    uint32_t len;
    char *line = prev_line(it, &len);
    if (!line)
      break;
    if (len > 0 && line[len - 1] == '\n')
      --len;
    if (len == 0)
      continue;
    c_indent = indent_real(line, len);
    auto is_end_set = kLangtoBlockStartsEnd.find(editor->lang.name);
    auto is_start_set = kLangtoBlockStartsStart.find(editor->lang.name);
    bool is_end = false;
    if (is_end_set != kLangtoBlockStartsEnd.end())
      for (auto end : is_end_set->second)
        if (ends_with(line, end)) {
          c_indent++;
          is_end = true;
          break;
        }
    if (!is_end && is_start_set != kLangtoBlockStartsStart.end())
      for (auto end : is_start_set->second)
        if (starts_with(line, end)) {
          c_indent++;
          break;
        }
    break;
  }
  free(it->buffer);
  free(it);
  return c_indent;
}

uint32_t IndentationEngine::set_indent(uint32_t row, int64_t new_indent) {
  std::shared_lock lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, row);
  if (!it)
    return 0;
  uint32_t len;
  char *line = next_line(it, &len);
  if (!line) {
    free(it->buffer);
    free(it);
    return 0;
  }
  if (len > 0 && line[len - 1] == '\n')
    --len;
  lock.unlock();
  if (new_indent <= 0)
    new_indent = 0;
  uint32_t ws_len = 0;
  while (ws_len < len && (line[ws_len] == ' ' || line[ws_len] == '\t'))
    ws_len++;
  std::string new_ws;
  if (indent == 1)
    new_ws.assign(new_indent, '\t');
  else
    new_ws.assign(new_indent * indent, ' ');
  Coord start = {row, 0};
  Coord end = {row, ws_len};
  edit_replace(editor, start, end, new_ws.c_str(), new_ws.length());
  free(it->buffer);
  free(it);
  return len - ws_len + (new_indent * indent);
}

uint32_t IndentationEngine::indent_line(uint32_t row) {
  std::shared_lock lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, row);
  if (!it)
    return 0;
  uint32_t len;
  char *line = next_line(it, &len);
  if (!line) {
    free(it->buffer);
    free(it);
    return 0;
  }
  lock.unlock();
  if (len > 0 && line[len - 1] == '\n')
    --len;
  uint32_t new_indent = indent_real(line, len) + 1;
  uint32_t ws_len = 0;
  while (ws_len < len && (line[ws_len] == ' ' || line[ws_len] == '\t'))
    ws_len++;
  std::string new_ws;
  if (indent == 1)
    new_ws.assign(new_indent, '\t');
  else
    new_ws.assign(new_indent * indent, ' ');
  edit_replace(editor, {row, 0}, {row, ws_len}, new_ws.c_str(),
               new_indent * indent);
  free(it->buffer);
  free(it);
  return (uint32_t)ABS((int64_t)ws_len - (new_indent * indent));
}

uint32_t IndentationEngine::dedent_line(uint32_t row) {
  std::shared_lock lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, row);
  if (!it)
    return 0;
  uint32_t len;
  char *line = next_line(it, &len);
  if (!line) {
    free(it->buffer);
    free(it);
    return 0;
  }
  lock.unlock();
  if (len > 0 && line[len - 1] == '\n')
    --len;
  int64_t new_indent = (int64_t)indent_real(line, len) - 1;
  if (new_indent < 0)
    new_indent = 0;
  uint32_t ws_len = 0;
  while (ws_len < len && (line[ws_len] == ' ' || line[ws_len] == '\t'))
    ws_len++;
  std::string new_ws;
  if (indent == 1)
    new_ws.assign(new_indent, '\t');
  else
    new_ws.assign(new_indent * indent, ' ');
  edit_replace(editor, {row, 0}, {row, ws_len}, new_ws.c_str(),
               new_indent * indent);
  free(it->buffer);
  free(it);
  return (uint32_t)ABS((int64_t)ws_len - (new_indent * indent));
}

void IndentationEngine::insert_new_line(Coord cursor) {
  std::string formatted;
  std::shared_lock lock(editor->knot_mtx);
  LineIterator *it = begin_l_iter(editor->root, cursor.row);
  if (!it)
    return;
  uint32_t len;
  char *line = next_line(it, &len);
  if (!line) {
    free(it->buffer);
    free(it);
    return;
  }
  lock.unlock();
  if (len > 0 && line[len - 1] == '\n')
    --len;
  if (cursor.col >= len) {
    auto is_end_full = kLangtoBlockEndsFull.find(editor->lang.name);
    auto is_end_start = kLangtoBlockEndsStart.find(editor->lang.name);
    bool end_matched = false;
    if (is_end_full != kLangtoBlockEndsFull.end())
      for (auto end : is_end_full->second)
        if (end == trim(line)) {
          cursor.col = set_indent(
              cursor.row, (int64_t)indent_expected(cursor.row) - (int64_t)1);
          end_matched = true;
          break;
        }
    if (!end_matched && is_end_start != kLangtoBlockEndsStart.end())
      for (auto end : is_end_start->second)
        if (starts_with(trim(line), end)) {
          cursor.col =
              set_indent(cursor.row, (int64_t)indent_expected(cursor.row) - 1);
          break;
        }
    lock.lock();
    free(it->buffer);
    free(it);
    it = begin_l_iter(editor->root, cursor.row);
    if (!it)
      return;
    line = next_line(it, &len);
    if (!line) {
      free(it->buffer);
      free(it);
      return;
    }
    if (len > 0 && line[len - 1] == '\n')
      --len;
    lock.unlock();
  }
  std::string ending = trim(std::string(line + cursor.col, len - cursor.col));
  std::string before = trim(std::string(line, cursor.col));
  uint32_t c_indent = indent_real(line, len);
  if (!ending.empty()) {
    bool ending_valid = false;
    bool starting_valid = false;
    auto is_end_full = kLangtoBlockEndsFull.find(editor->lang.name);
    auto is_end_start = kLangtoBlockEndsStart.find(editor->lang.name);
    auto is_end_set = kLangtoBlockStartsEnd.find(editor->lang.name);
    auto is_start_set = kLangtoBlockStartsStart.find(editor->lang.name);
    if (is_end_full != kLangtoBlockEndsFull.end())
      for (auto end : is_end_full->second)
        if (ending == end) {
          ending_valid = true;
          break;
        }
    if (!ending_valid && is_end_start != kLangtoBlockEndsStart.end())
      for (auto end : is_end_start->second)
        if (starts_with(ending, end)) {
          ending_valid = true;
          break;
        }
    if (is_end_set != kLangtoBlockStartsEnd.end())
      for (auto end : is_end_set->second)
        if (ends_with(before, end)) {
          c_indent++;
          starting_valid = true;
          break;
        }
    if (!starting_valid && is_start_set != kLangtoBlockStartsStart.end())
      for (auto end : is_start_set->second)
        if (starts_with(before, end)) {
          c_indent++;
          break;
        }
    if (ending_valid && starting_valid)
      ending = "\n" +
               (indent == 1 ? std::string(c_indent, '\t')
                            : std::string(c_indent * indent, ' ')) +
               ending;
    else if (ending_valid && c_indent)
      c_indent--;
  }
  auto is_end_set = kLangtoBlockStartsEnd.find(editor->lang.name);
  auto is_start_set = kLangtoBlockStartsStart.find(editor->lang.name);
  bool is_end = false;
  if (is_end_set != kLangtoBlockStartsEnd.end())
    for (auto end : is_end_set->second)
      if (ends_with(before, end)) {
        c_indent++;
        is_end = true;
        break;
      }
  if (!is_end && is_start_set != kLangtoBlockStartsStart.end())
    for (auto end : is_start_set->second)
      if (starts_with(before, end)) {
        c_indent++;
        break;
      }
  formatted = "\n" +
              (indent == 1 ? std::string(c_indent, '\t')
                           : std::string(c_indent * indent, ' ')) +
              ending;
  Coord new_cursor = {cursor.row + 1, c_indent * indent};
  edit_replace(editor, cursor, {cursor.row, len}, formatted.data(),
               formatted.size());
  editor->cursor = new_cursor;
  editor->cursor_preffered = UINT32_MAX;
  free(it->buffer);
  free(it);
  if (!editor->lsp || !editor->lsp->allow_formatting_on_type)
    return;
  for (char ch : editor->lsp->format_chars) {
    if (ch == '\n') {
      LineIterator *it = begin_l_iter(editor->root, editor->cursor.row);
      if (!it)
        return;
      uint32_t line_len;
      char *line = next_line(it, &line_len);
      if (!line) {
        free(it->buffer);
        free(it);
        return;
      }
      uint32_t col = utf8_offset_to_utf16(line, line_len, editor->cursor.col);
      free(it->buffer);
      free(it);
      int version = editor->lsp_version;
      json message = {
          {"jsonrpc", "2.0"},
          {"method", "textDocument/onTypeFormatting"},
          {"params",
           {{"textDocument", {{"uri", editor->uri}}},
            {"position", {{"line", editor->cursor.row}, {"character", col}}},
            {"ch", std::string(1, ch)},
            {"options",
             {{"tabSize", 2},
              {"insertSpaces", true},
              {"trimTrailingWhitespace", true},
              {"trimFinalNewlines", true}}}}}};
      LSPPending *pending = new LSPPending();
      pending->editor = editor;
      pending->method = "textDocument/onTypeFormatting";
      pending->callback = [version](Editor *editor, std::string, json message) {
        if (version != editor->lsp_version)
          return;
        auto &edits = message["result"];
        if (edits.is_array()) {
          std::vector<TextEdit> t_edits;
          t_edits.reserve(edits.size());
          for (auto &edit : edits) {
            TextEdit t_edit;
            t_edit.text = edit.value("newText", "");
            t_edit.start.row = edit["range"]["start"]["line"];
            t_edit.start.col = edit["range"]["start"]["character"];
            t_edit.end.row = edit["range"]["end"]["line"];
            t_edit.end.col = edit["range"]["end"]["character"];
            utf8_normalize_edit(editor, &t_edit);
            t_edits.push_back(t_edit);
          }
          apply_lsp_edits(editor, t_edits, false);
          ensure_scroll(editor);
        }
      };
      lsp_send(editor->lsp, message, pending);
      break;
    }
  }
}

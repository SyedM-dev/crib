#include "editor/decl.h"
#include "editor/editor.h"
#include "utils/utils.h"

void Editor::apply_lsp_edits(std::vector<TextEdit> edits, bool move) {
  if (!edits.size())
    return;
  TextEdit first = edits[0];
  Coord cursor = this->cursor;
  std::sort(
      edits.begin(), edits.end(),
      [](const TextEdit &a, const TextEdit &b) { return a.start > b.start; });
  for (const auto &edit : edits)
    this->edit_replace(edit.start, edit.end, edit.text.c_str(),
                       edit.text.size());
  if (move) {
    this->cursor = first.start;
    this->cursor = this->move_right(
        this->cursor, count_clusters(first.text.c_str(), first.text.size(), 0,
                                     first.text.size()));
  } else {
    if (cursor.row >= this->root->line_count) {
      this->cursor.row = this->root->line_count - 1;
      this->cursor.col = 0;
    } else {
      uint32_t len;
      line_to_byte(this->root, cursor.row, &len);
      len--;
      this->cursor.row = cursor.row;
      this->cursor.col = cursor.col < len ? cursor.col : len;
    }
  }
}

void Editor::lsp_handle(json msg) {
  if (msg.contains("method") &&
      msg["method"] == "textDocument/publishDiagnostics") {
    this->warnings.clear();
    json diagnostics = msg["params"]["diagnostics"];
    for (size_t i = 0; i < diagnostics.size(); i++) {
      json d = diagnostics[i];
      VWarn w;
      w.line = d["range"]["start"]["line"];
      w.start = d["range"]["start"]["character"];
      LineIterator *it = begin_l_iter(this->root, w.line);
      if (!it)
        continue;
      uint32_t len;
      char *line = next_line(it, &len);
      if (!line) {
        free(it->buffer);
        free(it);
        continue;
      }
      if (len > 0 && line[len - 1] == '\n')
        --len;
      w.start = utf16_offset_to_utf8(line, len, w.start);
      uint32_t end = d["range"]["end"]["character"];
      if (d["range"]["end"]["line"] == w.line)
        w.end = utf16_offset_to_utf8(line, len, end);
      free(it->buffer);
      free(it);
      std::string text = trim(d["message"].get<std::string>());
      w.text_full = text;
      auto pos = text.find('\n');
      w.text = (pos == std::string::npos) ? text : text.substr(0, pos);
      if (d.contains("source"))
        w.source = d["source"].get<std::string>();
      if (d.contains("code")) {
        w.code = "[";
        if (d["code"].is_string())
          w.code += d["code"].get<std::string>() + "] ";
        else if (d["code"].is_number())
          w.code += std::to_string(d["code"].get<int>()) + "] ";
        else
          w.code.clear();
        if (d.contains("codeDescription") &&
            d["codeDescription"].contains("href"))
          w.code += d["codeDescription"]["href"].get<std::string>();
      }
      if (d.contains("relatedInformation")) {
        json related = d["relatedInformation"];
        for (size_t j = 0; j < related.size(); j++) {
          json rel = related[j];
          std::string message = rel["message"].get<std::string>();
          auto pos = message.find('\n');
          message =
              (pos == std::string::npos) ? message : message.substr(0, pos);
          std::string uri = filename_from_path(
              percent_decode(rel["location"]["uri"].get<std::string>()));
          std::string row = std::to_string(
              rel["location"]["range"]["start"]["line"].get<int>());
          w.see_also.push_back(uri + ":" + row + ": " + message);
        }
      }
      w.type = 1;
      if (d.contains("severity"))
        w.type = d["severity"].get<int>();
      this->warnings.push_back(w);
    }
    std::sort(this->warnings.begin(), this->warnings.end());
    this->warnings_dirty = true;
  }
}

#include "editor/decl.h"
#include "editor/editor.h"

void apply_lsp_edits(Editor *editor, std::vector<TextEdit> edits, bool move) {
  if (!edits.size())
    return;
  TextEdit first = edits[0];
  Coord cursor = editor->cursor;
  std::sort(
      edits.begin(), edits.end(),
      [](const TextEdit &a, const TextEdit &b) { return a.start > b.start; });
  for (const auto &edit : edits)
    edit_replace(editor, edit.start, edit.end, edit.text.c_str(),
                 edit.text.size());
  if (move) {
    std::shared_lock lock(editor->knot_mtx);
    editor->cursor = first.start;
    editor->cursor =
        move_right_pure(editor, editor->cursor,
                        count_clusters(first.text.c_str(), first.text.size(), 0,
                                       first.text.size()));
  } else {
    if (cursor.row >= editor->root->line_count) {
      editor->cursor.row = editor->root->line_count - 1;
      editor->cursor.col = 0;
    } else {
      std::shared_lock lock(editor->knot_mtx);
      uint32_t len;
      line_to_byte(editor->root, cursor.row, &len);
      len--;
      editor->cursor.row = cursor.row;
      editor->cursor.col = cursor.col < len ? cursor.col : len;
    }
  }
}

void editor_lsp_handle(Editor *editor, json msg) {
  if (msg.contains("method") &&
      msg["method"] == "textDocument/publishDiagnostics") {
    std::unique_lock lock(editor->v_mtx);
    editor->warnings.clear();
    json diagnostics = msg["params"]["diagnostics"];
    for (size_t i = 0; i < diagnostics.size(); i++) {
      json d = diagnostics[i];
      VWarn w;
      // HACK: convert back to utf-8 but as this is only visually affecting it
      //       is not worth getting the line string from the rope.
      w.line = d["range"]["start"]["line"];
      w.start = d["range"]["start"]["character"];
      uint32_t end = d["range"]["end"]["character"];
      if (d["range"]["end"]["line"] == w.line)
        w.end = end;
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
      editor->warnings.push_back(w);
    }
    std::sort(editor->warnings.begin(), editor->warnings.end());
    editor->warnings_dirty = true;
  }
}

#include "editor/editor.h"

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
          std::string uri =
              percent_decode(rel["location"]["uri"].get<std::string>());
          auto pos2 = uri.find_last_of('/');
          if (pos2 != std::string::npos)
            uri = uri.substr(pos2 + 1);
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

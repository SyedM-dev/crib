#include "editor/editor.h"
#include "io/knot.h"
#include "io/sysio.h"
#include "lsp/lsp.h"
#include "main.h"
#include "utils/utils.h"

void completion_request(Editor *editor) {
  Coord hook = editor->cursor;
  word_boundaries(editor, editor->cursor, &hook.col, nullptr, nullptr, nullptr);
  LineIterator *it = begin_l_iter(editor->root, hook.row);
  char *line = next_line(it, nullptr);
  if (!line) {
    free(it->buffer);
    free(it);
    return;
  }
  hook.col = utf8_byte_offset_to_utf16(line, hook.col);
  editor->completion.hook = hook;
  LSPPending *pending = new LSPPending();
  pending->editor = editor;
  pending->method = "textDocument/completion";
  pending->callback = [line, it](Editor *editor, std::string, json message) {
    auto &session = editor->completion;
    std::unique_lock lock(session.mtx);
    session.active = true;
    session.items.clear();
    session.select = 0;
    std::vector<json> items_json;
    std::vector<char> end_chars_def;
    int insert_text_format = 1;
    if (message.contains("result")) {
      auto &result = message["result"];
      if (result.is_array()) {
        items_json = result.get<std::vector<json>>();
        session.complete = true;
      } else if (result.is_object() && result.contains("items")) {
        auto &list = result;
        items_json = list["items"].get<std::vector<json>>();
        session.complete = !list.value("isIncomplete", false);
        if (list.contains("itemDefaults")) {
          auto &defs = list["itemDefaults"];
          if (defs.contains("insertTextFormat"))
            insert_text_format = defs["insertTextFormat"].get<int>();
          if (defs.contains("textEdit"))
            if (defs["textEdit"].is_array())
              for (auto &c : defs["textEdit"]) {
                std::string str = c.get<std::string>();
                if (str.size() != 1)
                  continue;
                end_chars_def.push_back(str[0]);
              }
        }
      }
    }
    session.items.reserve(items_json.size());
    for (auto &item_json : items_json) {
      CompletionItem item;
      item.original = item_json;
      item.label = item_json.value("label", "");
      item.kind = item_json.value("kind", 0);
      if (item_json.contains("detail"))
        item.detail = item_json["detail"].get<std::string>();
      if (item_json.contains("documentation")) {
        if (item_json["documentation"].is_string()) {
          item.documentation = item_json["documentation"].get<std::string>();
        } else if (item_json["documentation"].contains("value")) {
          item.is_markup =
              item_json["documentation"]["kind"].get<std::string>() ==
              "markdown";
          item.documentation =
              item_json["documentation"]["value"].get<std::string>();
        }
      }
      if (item_json.contains("deprecated"))
        item.deprecated = item_json["deprecated"].get<bool>();
      auto tags = item_json.value("tags", std::vector<int>());
      for (auto tag : tags)
        if (tag == 1)
          item.deprecated = true;
      item.sort = item_json.value("sortText", item.label);
      item.filter = item_json.value("filterText", item.label);
      if (item_json.contains("preselect") && item_json["preselect"].get<bool>())
        session.select = session.items.size() - 1;
      TextEdit edit;
      if (item_json.contains("textEdit")) {
        auto &te = item_json["textEdit"];
        if (te.contains("newText")) {
          edit.text = te.value("newText", "");
          if (te.contains("replace")) {
            edit.start.row = te["replace"]["start"]["line"];
            edit.start.col = te["replace"]["start"]["character"];
            edit.end.row = te["replace"]["end"]["line"];
            edit.end.col = te["replace"]["end"]["character"];
          } else if (te.contains("insert")) {
            edit.start.row = te["insert"]["start"]["line"];
            edit.start.col = te["insert"]["start"]["character"];
            edit.end.row = te["insert"]["end"]["line"];
            edit.end.col = te["insert"]["end"]["character"];
          }
        } else {
          edit.text = te.value("newText", "");
          edit.start.row = te["range"]["start"]["line"];
          edit.start.col = te["range"]["start"]["character"];
          edit.end.row = te["range"]["end"]["line"];
          edit.end.col = te["range"]["end"]["character"];
        }
      } else if (item_json.contains("insertText")) {
        edit.text = item_json["insertText"].get<std::string>();
        edit.start = session.hook;
        uint32_t col = utf8_byte_offset_to_utf16(line, editor->cursor.col);
        edit.end = {editor->cursor.row, col};
      }
      item.edits.push_back(edit);
      if (item_json.contains("additionalTextEdits")) {
        for (auto &te : item_json["additionalTextEdits"]) {
          TextEdit edit;
          edit.text = te.value("newText", "");
          edit.start.row = te["range"]["start"]["line"];
          edit.start.col = te["range"]["start"]["character"];
          edit.end.row = te["range"]["end"]["line"];
          edit.end.col = te["range"]["end"]["character"];
          item.edits.push_back(edit);
        }
      }
      item.snippet = insert_text_format == 2;
      if (item_json.contains("insertTextFormat"))
        item.snippet = item_json["insertTextFormat"].get<int>() == 2;
      if (item_json.contains("commitCharacters"))
        for (auto &c : item_json["commitCharacters"])
          if (c.is_string() && c.get<std::string>().size() == 1)
            item.end_chars.push_back(c.get<std::string>()[0]);
      session.items.push_back(std::move(item));
      session.visible.push_back(session.items.size() - 1);
    }
    session.box.hidden = false;
    session.box.render_update();
    free(it->buffer);
    free(it);
  };
  uint32_t col = utf8_byte_offset_to_utf16(line, editor->cursor.col);
  json message = {
      {"jsonrpc", "2.0"},
      {"method", "textDocument/completion"},
      {"params",
       {{"textDocument", {{"uri", editor->uri}}},
        {"position", {{"line", editor->cursor.row}, {"character", col}}}}}};
  if (editor->completion.trigger > 0) {
    json context = {{"triggerKind", editor->completion.trigger}};
    if (editor->completion.trigger == 2 && editor->completion.trigger_char)
      context["triggerCharacter"] =
          std::string(1, *editor->completion.trigger_char);
    message["params"]["context"] = context;
  }
  lsp_send(editor->lsp, message, pending);
}

inline static std::string completion_prefix(Editor *editor) {
  Coord hook = editor->completion.hook;
  Coord cur = editor->cursor;
  if (hook.row != cur.row || cur.col < hook.col)
    return "";
  LineIterator *it = begin_l_iter(editor->root, hook.row);
  char *line = next_line(it, nullptr);
  if (!line) {
    free(it->buffer);
    free(it);
    return "";
  }
  uint32_t start = utf16_offset_to_utf8(line, hook.col);
  uint32_t end = editor->cursor.col;
  std::string prefix(line + start, end - start);
  free(it->buffer);
  free(it);
  return prefix;
}

void completion_filter(Editor *editor) {
  auto &session = editor->completion;
  std::string prefix = completion_prefix(editor);
  session.visible.clear();
  for (size_t i = 0; i < session.items.size(); ++i) {
    const auto &item = session.items[i];
    const std::string &key = item.filter.empty() ? item.label : item.filter;
    if (key.size() >= prefix.size() && key.substr(0, prefix.size()) == prefix)
      session.visible.push_back(i);
  }
  if (session.visible.empty()) {
    session.box.hidden = true;
    return;
  }
  session.box.hidden = false;
  session.box.render_update();
  bool found = false;
  for (int i : session.visible)
    if (i == session.select) {
      found = true;
      break;
    }
  if (!found)
    session.select = session.visible[0];
}

void handle_completion(Editor *editor, KeyEvent event) {
  if (!editor->lsp || !editor->lsp->allow_completion)
    return;
  if (mode != INSERT) {
    editor->completion.active = false;
    return;
  }
  if (event.key_type == KEY_PASTE) {
    editor->completion.active = false;
    return;
  } else if (event.key_type == KEY_CHAR) {
    char ch = *event.c;
    if (!editor->completion.active) {
      for (char c : editor->lsp->trigger_chars)
        if (c == ch) {
          editor->completion.trigger = 2;
          editor->completion.trigger_char = c;
          completion_request(editor);
          return;
        }
    } else {
      const auto &item = editor->completion.items[editor->completion.select];
      const std::vector<char> &end_chars =
          item.end_chars.empty() ? editor->lsp->end_chars : item.end_chars;
      for (char c : end_chars)
        if (c == ch) {
          complete_accept(editor);
          return;
        }
    }
    if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' ||
        ch >= '0' && ch <= '9' || ch == '_') {
      if (editor->completion.active) {
        if (editor->completion.complete)
          completion_filter(editor);
        else
          completion_request(editor);
      } else {
        editor->completion.trigger = 3;
        completion_request(editor);
      }
    } else if (ch == CTRL('\\')) {
      if (editor->completion.active) {
        complete_accept(editor);
      } else {
        editor->completion.trigger = 1;
        completion_request(editor);
      }
    } else if (ch == CTRL(']')) {
      if (editor->completion.active)
        complete_next(editor);
    } else if (ch == CTRL('[')) {
      if (editor->completion.active)
        complete_prev(editor);
    } else if (ch == 0x7F || ch == 0x08) {
      if (editor->completion.complete)
        completion_filter(editor);
      else
        completion_request(editor);
    } else {
      editor->completion.active = false;
    }
  } else if (event.key_type == KEY_MOUSE && event.mouse_modifier == 0) {
    auto &box = editor->completion.box;
    Coord normalized = {event.mouse_y - box.position.row,
                        event.mouse_x - box.position.col};
    if (normalized.row >= 0 && normalized.row < box.size.row &&
        normalized.col >= 0 && normalized.col < box.size.col) {
      uint8_t idx = 0;
      /* todo: calculate idx based on scroll and mouse position */
      complete_select(editor, idx);
    }
  }
}

void completion_resolve_doc(Editor *editor) {
  std::unique_lock lock(editor->completion.mtx);
  auto &item = editor->completion.items[editor->completion.select];
  if (item.documentation)
    return;
  item.documentation = "";
  LSPPending *pending = new LSPPending();
  pending->editor = editor;
  pending->method = "completionItem/resolve";
  pending->callback = [](Editor *editor, std::string, json message) {
    std::unique_lock lock(editor->completion.mtx);
    auto &item = editor->completion.items[editor->completion.select];
    if (message.contains("documentation"))
      item.documentation = message["documentation"].get<std::string>();
    else
      item.documentation = "";
  };
  json message = {{"jsonrpc", "2.0"},
                  {"method", "completionItem/resolve"},
                  {"params", item.original}};
  lsp_send(editor->lsp, message, pending);
}

void complete_accept(Editor *editor) {
  if (!editor->completion.active || editor->completion.box.hidden)
    return;
  auto &item = editor->completion.items[editor->completion.select];
  apply_lsp_edits(editor, item.edits);
  editor->completion.active = false;
}

inline static int visible_index(const CompletionSession &s) {
  for (size_t i = 0; i < s.visible.size(); ++i)
    if (s.visible[i] == s.select)
      return (int)i;
  return -1;
}

void complete_next(Editor *editor) {
  auto &s = editor->completion;
  if (!s.active || s.box.hidden || s.visible.empty())
    return;
  int vi = visible_index(s);
  if (vi < 0)
    vi = 0;
  else
    vi = (vi + 1) % s.visible.size();
  s.select = s.visible[vi];
  completion_resolve_doc(editor);
}

void complete_prev(Editor *editor) {
  auto &s = editor->completion;
  if (!s.active || s.box.hidden || s.visible.empty())
    return;
  int vi = visible_index(s);
  if (vi < 0)
    vi = 0;
  else
    vi = (vi + s.visible.size() - 1) % s.visible.size();
  s.select = s.visible[vi];
  completion_resolve_doc(editor);
}

void complete_select(Editor *editor, uint8_t index) {
  editor->completion.select = index;
  complete_accept(editor);
}

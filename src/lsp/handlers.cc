#include "lsp/lsp.h"

Queue<LSPOpenRequest> lsp_open_queue;

void request_add_to_lsp(Language language, Editor *editor) {
  lsp_open_queue.push({language, editor});
}

void add_to_lsp(Language language, Editor *editor) {
  std::shared_ptr<LSPInstance> lsp = get_or_init_lsp(language.lsp_name);
  if (!lsp)
    return;
  std::unique_lock lock(lsp->mtx);
  if (editor->lsp == lsp)
    return;
  lsp->editors.push_back(editor);
  lsp->open_queue.push({language, editor});
  lock.unlock();
}

void open_editor(std::shared_ptr<LSPInstance> lsp,
                 std::pair<Language, Editor *> entry) {
  Language language = entry.first;
  Editor *editor = entry.second;
  if (editor->lsp == lsp)
    return;
  editor->lsp = lsp;
  std::unique_lock lock3(editor->knot_mtx);
  char *buf = read(editor->root, 0, editor->root->char_count);
  std::string text(buf);
  free(buf);
  json message = {{"jsonrpc", "2.0"},
                  {"method", "textDocument/didOpen"},
                  {"params",
                   {{"textDocument",
                     {{"uri", editor->uri},
                      {"languageId", language.name},
                      {"version", 1},
                      {"text", text}}}}}};
  lock3.unlock();
  lsp_send(lsp, message, nullptr);
}

static std::string find_lsp_id(std::shared_ptr<LSPInstance> needle) {
  for (const auto &[id, lsp] : active_lsps)
    if (lsp == needle)
      return id;
  return "";
}

void remove_from_lsp(Editor *editor) {
  auto lsp = editor->lsp;
  if (!lsp)
    return;
  std::unique_lock lock1(lsp->mtx);
  lsp->editors.erase(
      std::remove(lsp->editors.begin(), lsp->editors.end(), editor),
      lsp->editors.end());
  lock1.unlock();
  std::unique_lock lock2(editor->lsp_mtx);
  editor->lsp = nullptr;
  lock2.unlock();
  json message = {{"jsonrpc", "2.0"},
                  {"method", "textDocument/didClose"},
                  {"params", {{"textDocument", {{"uri", editor->uri}}}}}};
  lsp_send(lsp, message, nullptr);
  std::string lsp_id = find_lsp_id(lsp);
  if (!lsp_id.empty() && lsp->editors.empty())
    close_lsp(lsp_id);
}

void lsp_handle(std::shared_ptr<LSPInstance>, json message) {
  std::string method = message.value("method", "");
  if (method == "window/showMessage") {
    if (message.contains("params")) {
      auto &p = message["params"];
      if (p.contains("message"))
        log("%s\n", p["message"].get<std::string>().c_str());
    }
  } else if (method == "window/logMessage") {
    if (message.contains("params")) {
      auto &p = message["params"];
      if (p.contains("message"))
        log("%s\n", p["message"].get<std::string>().c_str());
    }
  }
}

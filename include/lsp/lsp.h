#ifndef LSP_H
#define LSP_H

#include "editor/editor.h"
#include "pch.h"
#include "utils/utils.h"

struct LSPPending {
  Editor *editor = nullptr;
  std::function<void(Editor *, const json &)> callback;
};

// TODO: Defer any editor mutation to main thread to get rid of
//       all mutex locks on the editor rope.
// struct LSPPendingResponse {
//   LSPPending *pending = nullptr;
//   json message;
// };

struct LSPOpenRequest {
  Language language;
  Editor *editor;
};

struct LSPInstance {
  std::shared_mutex mtx;
  const LSP *lsp;
  std::string root_dir;
  int pid{-1};
  int stdin_fd{-1};
  int stdout_fd{-1};
  std::atomic<bool> initialized = false;
  std::atomic<bool> exited = false;
  bool incremental_sync = false;
  bool allow_hover = false;
  bool allow_completion = false;
  bool allow_resolve = false;
  bool allow_formatting = false;
  bool allow_formatting_on_type = false;
  bool is_utf8 = false;
  std::vector<char> format_chars;
  std::vector<char> trigger_chars;
  std::vector<char> end_chars;
  uint32_t last_id = 0;
  Queue<json> inbox;
  Queue<json> outbox;
  Queue<std::pair<Language, Editor *>> open_queue;
  std::unordered_map<uint32_t, LSPPending *> pending;
  std::vector<Editor *> editors;
};

extern std::shared_mutex active_lsps_mtx;
extern std::unordered_map<std::string, std::shared_ptr<LSPInstance>>
    active_lsps;
extern Queue<LSPOpenRequest> lsp_open_queue;

static json client_capabilities = {
    {"general", {{"positionEncodings", {"utf-16"}}}},
    {"textDocument",
     {{"publishDiagnostics", {{"relatedInformation", true}}},
      {"hover", {{"contentFormat", {"markdown", "plaintext"}}}},
      {"formatting", {{"dynamicRegistration", false}}},
      {"onTypeFormatting", {{"dynamicRegistration", false}}},
      {"completion",
       {{"completionItem",
         {{"commitCharactersSupport", true},
          {"dynamicRegistration", false},
          {"snippetSupport", true},
          {"documentationFormat", {"markdown", "plaintext"}},
          {"resolveSupport", {{"properties", {"documentation"}}}},
          {"insertReplaceSupport", true},
          {"labelDetailsSupport", true},
          {"insertTextModeSupport", {{"valueSet", {1, 2}}}},
          {"deprecatedSupport", true}}},
        {"completionItemKind",
         {{"valueSet", {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
                        14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25}}}},
        {"contextSupport", true},
        {"insertTextMode", 1}}}}}};

void lsp_send(std::shared_ptr<LSPInstance> lsp, json message,
              LSPPending *pending);
void lsp_worker();

std::shared_ptr<LSPInstance> get_or_init_lsp(std::string lsp_id);
void clean_lsp(std::shared_ptr<LSPInstance> lsp, std::string lsp_id);
void close_lsp(std::string lsp_id);
std::optional<json> read_lsp_message(int fd);

void open_editor(std::shared_ptr<LSPInstance> lsp,
                 std::pair<Language, Editor *> entry);
void request_add_to_lsp(Language language, Editor *editor);
void add_to_lsp(Language language, Editor *editor);
void remove_from_lsp(Editor *editor);
void lsp_handle(std::shared_ptr<LSPInstance> lsp, json message);

#endif

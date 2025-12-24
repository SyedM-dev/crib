#ifndef LSP_H
#define LSP_H

#include "./editor.h"
#include "./pch.h"
#include "utils.h"

struct LSP {
  const char *command;
  std::vector<const char *> args;
};

struct LSPPending {
  std::string method;
  Editor *editor = nullptr;

  std::function<void(Editor *, std::string, json)> callback;
};

struct LSPOpenRequest {
  Language language;
  Editor *editor;
};

struct LSPInstance {
  std::shared_mutex mtx;
  LSP *lsp;
  std::string root_dir;
  int pid{-1};
  int stdin_fd{-1};
  int stdout_fd{-1};
  bool initialized = false;
  uint32_t last_id = 0;
  Queue<json> inbox;
  Queue<json> outbox;
  std::unordered_map<uint32_t, LSPPending *> pending;
  std::vector<Editor *> editors;
};

extern std::shared_mutex active_lsps_mtx;
extern std::unordered_map<uint8_t, LSPInstance *> active_lsps;
extern std::unordered_map<uint8_t, LSP> lsp_map;

void lsp_worker();
void lsp_handle(LSPInstance *lsp, json message);

LSPInstance *get_or_init_lsp(uint8_t lsp_id);
void close_lsp(uint8_t lsp_id);

void request_add_to_lsp(Language language, Editor *editor);
void add_to_lsp(Language language, Editor *editor);
void remove_from_lsp(Editor *editor);

void lsp_send(LSPInstance *lsp, json message, LSPPending *pending);

#endif

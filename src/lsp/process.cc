#include "config.h"
#include "lsp/lsp.h"

static bool init_lsp(std::shared_ptr<LSPInstance> lsp) {
  int in_pipe[2];
  int out_pipe[2];
  if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1) {
    perror("pipe");
    return false;
  }
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return false;
  }
  if (pid == 0) {
    dup2(in_pipe[0], STDIN_FILENO);
    dup2(out_pipe[1], STDOUT_FILENO);
#ifdef __clang__
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull >= 0) {
      dup2(devnull, STDERR_FILENO);
      close(devnull);
    }
#else
    int log = open("/tmp/lsp.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log >= 0) {
      dup2(log, STDERR_FILENO);
      close(log);
    }
#endif
    close(in_pipe[0]);
    close(in_pipe[1]);
    close(out_pipe[0]);
    close(out_pipe[1]);
    execvp(lsp->lsp->command, (char *const *)(lsp->lsp->args.data()));
    perror("execvp");
    _exit(127);
  }
  lsp->pid = pid;
  lsp->stdin_fd = in_pipe[1];
  lsp->stdout_fd = out_pipe[0];
  close(in_pipe[0]);
  close(out_pipe[1]);
  return true;
}

std::shared_ptr<LSPInstance> get_or_init_lsp(uint8_t lsp_id) {
  std::unique_lock lock(active_lsps_mtx);
  auto it = active_lsps.find(lsp_id);
  if (it == active_lsps.end()) {
    auto map_it = kLsps.find(lsp_id);
    if (map_it == kLsps.end())
      return nullptr;
    std::shared_ptr<LSPInstance> lsp = std::make_shared<LSPInstance>();
    lsp->lsp = &map_it->second;
    if (!init_lsp(lsp))
      return nullptr;
    LSPPending *pending = new LSPPending();
    pending->method = "initialize";
    pending->editor = nullptr;
    pending->callback = [lsp](Editor *, std::string, json msg) {
      if (msg.contains("result") && msg["result"].contains("capabilities")) {
        auto &caps = msg["result"]["capabilities"];
        if (caps.contains("textDocumentSync")) {
          auto &sync = caps["textDocumentSync"];
          if (sync.is_number()) {
            int change_type = sync.get<int>();
            lsp->incremental_sync = (change_type == 2);
          } else if (sync.is_object() && sync.contains("change")) {
            int change_type = sync["change"].get<int>();
            lsp->incremental_sync = (change_type == 2);
          }
        }
        if (caps.contains("hoverProvider"))
          lsp->allow_hover = caps["hoverProvider"].get<bool>();
        else
          lsp->allow_hover = false;
        if (caps.contains("completionProvider")) {
          lsp->allow_completion = true;
          if (caps["completionProvider"].contains("resolveProvider"))
            lsp->allow_resolve =
                caps["completionProvider"]["resolveProvider"].get<bool>();
          if (caps["completionProvider"].contains("triggerCharacters")) {
            auto &chars = caps["completionProvider"]["triggerCharacters"];
            if (chars.is_array()) {
              for (auto &c : chars) {
                std::string str = c.get<std::string>();
                if (str.size() != 1)
                  continue;
                lsp->trigger_chars.push_back(str[0]);
              }
            }
          }
          if (caps["completionProvider"].contains("allCommitCharacters")) {
            auto &chars = caps["completionProvider"]["allCommitCharacters"];
            if (chars.is_array()) {
              for (auto &c : chars) {
                std::string str = c.get<std::string>();
                if (str.size() != 1)
                  continue;
                lsp->end_chars.push_back(str[0]);
              }
            }
          }
        }
      }
      lsp->initialized = true;
      json initialized = {{"jsonrpc", "2.0"},
                          {"method", "initialized"},
                          {"params", json::object()}};
      lsp_send(lsp, initialized, nullptr);
    };
    json init_message = {
        {"jsonrpc", "2.0"},
        {"method", "initialize"},
        {"params",
         {{"processId", getpid()},
          {"rootUri", "file://" + percent_encode(path_abs("."))},
          {"capabilities", client_capabilities}}}};
    lsp_send(lsp, init_message, pending);
    active_lsps[lsp_id] = lsp;
    return lsp;
  }
  return it->second;
}

void close_lsp(uint8_t lsp_id) {
  std::shared_lock active_lsps_lock(active_lsps_mtx);
  auto it = active_lsps.find(lsp_id);
  if (it == active_lsps.end())
    return;
  std::shared_ptr<LSPInstance> lsp = it->second;
  active_lsps_lock.unlock();
  lsp->exited = true;
  lsp->initialized = false;
  LSPPending *shutdown_pending = new LSPPending();
  shutdown_pending->method = "shutdown";
  shutdown_pending->callback = [lsp, lsp_id](Editor *, std::string, json) {
    json exit = {{"jsonrpc", "2.0"}, {"method", "exit"}};
    lsp_send(lsp, exit, nullptr);
  };
  json shutdown = {{"jsonrpc", "2.0"}, {"method", "shutdown"}};
  lsp_send(lsp, shutdown, shutdown_pending);
  std::thread t([lsp, lsp_id] {
    std::this_thread::sleep_for(100ms);
    std::unique_lock active_lsps_lock(active_lsps_mtx);
    std::unique_lock lock(lsp->mtx);
    if (lsp->pid != -1 && kill(lsp->pid, 0) == 0)
      kill(lsp->pid, SIGKILL);
    waitpid(lsp->pid, nullptr, 0);
    close(lsp->stdin_fd);
    close(lsp->stdout_fd);
    for (auto &kv : lsp->pending)
      delete kv.second;
    for (auto &editor : lsp->editors) {
      std::unique_lock editor_lock(editor->lsp_mtx);
      editor->lsp = nullptr;
    }
    active_lsps.erase(lsp_id);
  });
  t.detach();
}

void clean_lsp(std::shared_ptr<LSPInstance> lsp, uint8_t lsp_id) {
  for (auto &kv : lsp->pending)
    delete kv.second;
  lsp->pid = -1;
  close(lsp->stdin_fd);
  close(lsp->stdout_fd);
  for (auto &editor : lsp->editors) {
    std::unique_lock editor_lock(editor->lsp_mtx);
    editor->lsp = nullptr;
  }
  active_lsps.erase(lsp_id);
}

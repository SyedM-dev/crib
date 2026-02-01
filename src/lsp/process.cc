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
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull >= 0) {
      dup2(devnull, STDERR_FILENO);
      close(devnull);
    }
    close(in_pipe[0]);
    close(in_pipe[1]);
    close(out_pipe[0]);
    close(out_pipe[1]);
    std::vector<char *> argv;
    argv.push_back(const_cast<char *>(lsp->lsp->command.c_str()));
    for (auto &arg : lsp->lsp->args)
      argv.push_back(const_cast<char *>(arg.c_str()));
    argv.push_back(nullptr);
    execvp(lsp->lsp->command.c_str(), argv.data());
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

std::shared_ptr<LSPInstance> get_or_init_lsp(std::string lsp_id) {
  std::unique_lock lock(active_lsps_mtx);
  auto it = active_lsps.find(lsp_id);
  if (it == active_lsps.end()) {
    auto map_it = lsps.find(lsp_id);
    if (map_it == lsps.end())
      return nullptr;
    std::shared_ptr<LSPInstance> lsp = std::make_shared<LSPInstance>();
    lsp->lsp = &map_it->second;
    if (!init_lsp(lsp))
      return nullptr;
    LSPPending *pending = new LSPPending();
    pending->editor = nullptr;
    pending->callback = [lsp, lsp_id](Editor *, const json &msg) {
      if (msg.contains("result") && msg["result"].contains("capabilities")) {
        auto &caps = msg["result"]["capabilities"];
        // if (caps.contains("positionEncoding")) {
        //   std::string s = caps["positionEncoding"].get<std::string>();
        //   if (s == "utf-8")
        //     lsp->is_utf8 = true;
        //   log("Lsp name: %s, supports: %s", lsp->lsp->command.c_str(),
        //       s.c_str());
        // }
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
        lsp->allow_formatting = caps.value("documentFormattingProvider", false);
        if (lsp_id != "lua-language-server" /* Lua ls gives terrible ontype
                                               formatting so disable */
            && caps.contains("documentOnTypeFormattingProvider")) {
          auto &fmt = caps["documentOnTypeFormattingProvider"];
          if (fmt.is_object()) {
            if (fmt.contains("firstTriggerCharacter")) {
              std::string s = fmt["firstTriggerCharacter"].get<std::string>();
              if (s.size() == 1)
                lsp->format_chars.push_back(s[0]);
            }
            if (fmt.contains("moreTriggerCharacter")) {
              for (auto &c : fmt["moreTriggerCharacter"]) {
                std::string s = c.get<std::string>();
                if (s.size() == 1)
                  lsp->format_chars.push_back(s[0]);
              }
            }
            lsp->allow_formatting_on_type = true;
          } else if (fmt.is_boolean()) {
            lsp->allow_formatting_on_type = fmt.get<bool>();
          }
        }
        if (caps.contains("hoverProvider")) {
          auto &hover = caps["hoverProvider"];
          lsp->allow_hover =
              hover.is_boolean() ? hover.get<bool>() : hover.is_object();
        } else {
          lsp->allow_hover = false;
        }
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

void close_lsp(std::string lsp_id) {
  std::shared_ptr<LSPInstance> lsp;
  {
    std::shared_lock lock(active_lsps_mtx);
    auto it = active_lsps.find(lsp_id);
    if (it == active_lsps.end())
      return;
    lsp = it->second;
  }
  if (!lsp || lsp->pid == -1 || lsp->exited)
    return;
  lsp->exited = true;
  lsp->initialized = false;
  auto send_raw = [&](const json &msg) {
    std::string payload = msg.dump();
    std::string header =
        "Content-Length: " + std::to_string(payload.size()) + "\r\n\r\n";
    std::string out = header + payload;
    const char *ptr = out.data();
    size_t remaining = out.size();
    while (remaining > 0) {
      ssize_t n = write(lsp->stdin_fd, ptr, remaining);
      if (n <= 0) {
        if (errno == EINTR)
          continue;
        break;
      }
      ptr += n;
      remaining -= n;
    }
  };
  json shutdown = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "shutdown"}};
  send_raw(shutdown);
  {
    pollfd pfd{lsp->stdout_fd, POLLIN, 0};
    int timeout_ms = 300;
    if (poll(&pfd, 1, timeout_ms) > 0) {
      auto msg = read_lsp_message(lsp->stdout_fd);
      (void)msg;
    }
  }
  json exit_msg = {{"jsonrpc", "2.0"}, {"method", "exit"}};
  send_raw(exit_msg);
  const int max_wait_ms = 500;
  int waited = 0;
  while (waited < max_wait_ms) {
    int status;
    pid_t res = waitpid(lsp->pid, &status, WNOHANG);
    if (res == lsp->pid)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    waited += 10;
  }
  if (kill(lsp->pid, 0) == 0) {
    kill(lsp->pid, SIGKILL);
    waitpid(lsp->pid, nullptr, 0);
  }
  close(lsp->stdin_fd);
  close(lsp->stdout_fd);
  {
    std::unique_lock lock(lsp->mtx);
    for (auto &kv : lsp->pending)
      delete kv.second;
    lsp->pending.clear();
  }
  for (auto &editor : lsp->editors) {
    std::unique_lock editor_lock(editor->lsp_mtx);
    editor->lsp = nullptr;
  }
  {
    std::unique_lock lock(active_lsps_mtx);
    active_lsps.erase(lsp_id);
  }
}

void clean_lsp(std::shared_ptr<LSPInstance> lsp, std::string lsp_id) {
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

#include "../include/lsp.h"
#include "../include/maps.h"
#include <cmath>
#include <fcntl.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

std::shared_mutex active_lsps_mtx;
std::unordered_map<uint8_t, std::shared_ptr<LSPInstance>> active_lsps;

Queue<LSPOpenRequest> lsp_open_queue;

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
        if (caps.contains("completionProvider"))
          lsp->allow_completion = true;
      }
      lsp->initialized = true;
      json initialized = {{"jsonrpc", "2.0"},
                          {"method", "initialized"},
                          {"params", json::object()}};
      lsp_send(lsp, initialized, nullptr);
      while (!lsp->open_queue.empty()) {
        std::pair<Language, Editor *> request;
        lsp->open_queue.pop(request);
        open_editor(lsp, request);
      }
    };
    json init_message = {
        {"jsonrpc", "2.0"},
        {"method", "initialize"},
        {"params",
         {{"processId", getpid()},
          {"rootUri", "file://" + percent_encode(path_abs("."))},
          {"capabilities",
           {{"textDocument",
             {{"publishDiagnostics", {{"relatedInformation", true}}},
              {"hover", {{"contentFormat", {"markdown", "plaintext"}}}},
              {"completion",
               {{"completionItem",
                 {{"snippetSupport", true},
                  {"documentationFormat", {"markdown", "plaintext"}},
                  {"resolveSupport",
                   {{"properties", {"documentation", "detail"}}}},
                  {"insertReplaceSupport", true},
                  {"labelDetailsSupport", true},
                  {"insertTextModeSupport", {{"valueSet", json::array({1})}}}}},
                {"completionItemKind",
                 {{"valueSet", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}}}},
                {"contextSupport", true},
                {"insertTextMode", 1}}}}}}}}}};
    lsp_send(lsp, init_message, pending);
    active_lsps[lsp_id] = lsp;
    return lsp;
  }
  return it->second;
}

void lsp_send(std::shared_ptr<LSPInstance> lsp, json message,
              LSPPending *pending) {
  if (!lsp || lsp->stdin_fd == -1)
    return;
  std::unique_lock lock(lsp->mtx);
  if (pending) {
    message["id"] = lsp->last_id++;
    uint32_t id = message["id"].get<uint32_t>();
    lsp->pending[id] = pending;
  }
  lsp->outbox.push(message);
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

static std::optional<json> read_lsp_message(int fd) {
  std::string header;
  char c;
  while (true) {
    ssize_t n = read(fd, &c, 1);
    if (n <= 0)
      return std::nullopt;
    header.push_back(c);
    if (header.size() >= 4 && header.substr(header.size() - 4) == "\r\n\r\n")
      break;
  }
  size_t pos = header.find("Content-Length:");
  if (pos == std::string::npos)
    return std::nullopt;
  pos += strlen("Content-Length:");
  while (pos < header.size() && std::isspace(header[pos]))
    pos++;
  size_t end = pos;
  while (end < header.size() && std::isdigit(header[end]))
    end++;
  size_t len = std::stoul(header.substr(pos, end - pos));
  std::string body(len, '\0');
  size_t got = 0;
  while (got < len) {
    ssize_t n = read(fd, &body[got], len - got);
    if (n <= 0)
      return std::nullopt;
    got += n;
  }
  return json::parse(body);
}

static Editor *editor_for_uri(std::shared_ptr<LSPInstance> lsp,
                              std::string uri) {
  if (uri.empty())
    return nullptr;
  for (auto &editor : lsp->editors)
    if (editor->uri == uri)
      return editor;
  return nullptr;
}

static void clean_lsp(std::shared_ptr<LSPInstance> lsp, uint8_t lsp_id) {
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

void lsp_worker() {
  LSPOpenRequest request;
  while (lsp_open_queue.pop(request))
    add_to_lsp(request.language, request.editor);
  std::unique_lock active_lsps_lock(active_lsps_mtx);
  for (auto &kv : active_lsps) {
    std::shared_ptr<LSPInstance> lsp = kv.second;
    std::unique_lock lock(lsp->mtx);
    int status;
    pid_t res = waitpid(lsp->pid, &status, WNOHANG);
    if (res == lsp->pid) {
      clean_lsp(lsp, kv.first);
      return;
    }
    while (!lsp->outbox.empty()) {
      json message = lsp->outbox.front();
      std::string m = message.value("method", "");
      if (lsp->exited) {
        if (m != "exit" && m != "shutdown") {
          lsp->outbox.pop(message);
          continue;
        }
      }
      if (!lsp->initialized) {
        if (m != "initialize" && m != "exit" && m != "shutdown")
          break;
      }
      lsp->outbox.pop(message);
      std::string payload = message.dump();
      std::string header =
          "Content-Length: " + std::to_string(payload.size()) + "\r\n\r\n";
      std::string out = header + payload;
      const char *ptr = out.data();
      size_t remaining = out.size();
      while (remaining > 0) {
        int status;
        pid_t res = waitpid(lsp->pid, &status, WNOHANG);
        if (res == lsp->pid) {
          clean_lsp(lsp, kv.first);
          return;
        }
        ssize_t written = write(lsp->stdin_fd, ptr, remaining);
        if (written == 0)
          break;
        else if (written == -1) {
          if (errno == EINTR)
            continue;
          perror("write");
          clean_lsp(lsp, kv.first);
          return;
        } else {
          ptr += written;
          remaining -= written;
        }
      }
    }
    pollfd pfd{lsp->stdout_fd, POLLIN | POLLHUP | POLLERR, 0};
    int r = poll(&pfd, 1, 0);
    if (r > 0 && pfd.revents & (POLLHUP | POLLERR)) {
      clean_lsp(lsp, kv.first);
      return;
    }
    while ((r = poll(&pfd, 1, 0) > 0)) {
      if (r > 0 && pfd.revents & (POLLHUP | POLLERR)) {
        clean_lsp(lsp, kv.first);
        return;
      }
      auto msg = read_lsp_message(lsp->stdout_fd);
      if (!msg)
        break;
      if (msg->contains("id")) {
        uint32_t id = msg->at("id").get<uint32_t>();
        auto it = lsp->pending.find(id);
        if (it != lsp->pending.end()) {
          LSPPending *pend = it->second;
          lock.unlock();
          if (pend->callback)
            pend->callback(pend->editor, pend->method, *msg);
          delete pend;
          lock.lock();
          lsp->pending.erase(it);
        }
      } else if (msg->contains("method")) {
        std::string uri;
        if (msg->contains("params")) {
          auto &p = (*msg)["params"];
          if (p.contains("textDocument") && p["textDocument"].contains("uri"))
            uri = p["textDocument"]["uri"].get<std::string>();
          else if (p.contains("uri"))
            uri = p["uri"].get<std::string>();
        }
        Editor *ed = editor_for_uri(lsp, uri);
        lock.unlock();
        if (ed)
          editor_lsp_handle(ed, *msg);
        else
          lsp_handle(lsp, *msg);
        lock.lock();
      }
    }
  }
}

void request_add_to_lsp(Language language, Editor *editor) {
  lsp_open_queue.push({language, editor});
}

void add_to_lsp(Language language, Editor *editor) {
  std::shared_ptr<LSPInstance> lsp = get_or_init_lsp(language.lsp_id);
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
  std::unique_lock lock2(editor->lsp_mtx);
  editor->lsp = lsp;
  lock2.unlock();
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

static uint8_t find_lsp_id(std::shared_ptr<LSPInstance> needle) {
  for (const auto &[id, lsp] : active_lsps)
    if (lsp == needle)
      return id;
  return 0;
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
  uint8_t lsp_id = find_lsp_id(lsp);
  if (lsp_id && lsp->editors.empty())
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

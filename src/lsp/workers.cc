#include "lsp/lsp.h"

std::shared_mutex active_lsps_mtx;
std::unordered_map<std::string, std::shared_ptr<LSPInstance>> active_lsps;

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

std::optional<json> read_lsp_message(int fd) {
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
    if (lsp->initialized) {
      std::pair<Language, Editor *> request;
      while (lsp->open_queue.pop(request)) {
        lock.unlock();
        open_editor(lsp, request);
        lock.lock();
      }
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

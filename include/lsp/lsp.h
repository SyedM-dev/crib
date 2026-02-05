#ifndef LSP_H
#define LSP_H

#include "editor/editor.h"
#include "pch.h"
#include "utils/utils.h"

#define LSP_TIMEOUT 3000

namespace lsp {
extern std::mutex lsp_mutex;
extern std::unordered_map<std::string, std::unique_ptr<LSPInstance>>
    active_lsps;
extern Queue<std::string> need_opening;
extern std::unordered_set<std::string> opened;
extern std::vector<Editor *> new_editors;
} // namespace lsp

void lsp_worker();

static const json client_capabilities = {
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

struct LSPMessage {
  Editor *editor = nullptr;
  json message;
  std::function<void(const LSPMessage &)> callback;
};

struct LSPInstance {
  const LSP *lsp_info;
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
  std::unordered_map<uint32_t, std::unique_ptr<LSPMessage>> pending;
  std::vector<std::unique_ptr<LSPMessage>> lsp_response_queue;
  std::vector<Editor *> editors;

  LSPInstance(std::string lsp_id) {
    lsp_info = &lsps[lsp_id];
    if (!init_process()) {
      exited = true;
      return;
    }
    json initialize_message = {
        {"jsonrpc", "2.0"},
        {"id", ++last_id},
        {"method", "initialize"},
        {"params",
         {{"processId", getpid()},
          {"rootUri", "file://" + percent_encode(path_abs("."))},
          {"capabilities", client_capabilities}}}};
    send_raw(initialize_message);
    pollfd pfd{stdout_fd, POLLIN, 0};
    poll(&pfd, 1, LSP_TIMEOUT);
    if (!(pfd.revents & POLLIN)) {
      exited = true;
      return;
    }
    json response = *read_lsp_message();
    if (response.contains("result") &&
        response["result"].contains("capabilities")) {
      auto &caps = response["result"]["capabilities"];
      // if (caps.contains("positionEncoding")) {
      //   std::string s = caps["positionEncoding"].get<std::string>();
      //   if (s == "utf-8")
      //     is_utf8 = true;
      //   log("Lsp name: %s, supports: %s", lsp->command.c_str(),
      //       s.c_str());
      // }
      if (caps.contains("textDocumentSync")) {
        auto &sync = caps["textDocumentSync"];
        if (sync.is_number()) {
          int change_type = sync.get<int>();
          incremental_sync = (change_type == 2);
        } else if (sync.is_object() && sync.contains("change")) {
          int change_type = sync["change"].get<int>();
          incremental_sync = (change_type == 2);
        }
      }
      allow_formatting = caps.value("documentFormattingProvider", false);
      if (lsp_id != "lua-language-server" /* Lua ls gives terrible ontype
                                             formatting so disable */
          && caps.contains("documentOnTypeFormattingProvider")) {
        auto &fmt = caps["documentOnTypeFormattingProvider"];
        if (fmt.is_object()) {
          if (fmt.contains("firstTriggerCharacter")) {
            std::string s = fmt["firstTriggerCharacter"].get<std::string>();
            if (s.size() == 1)
              format_chars.push_back(s[0]);
          }
          if (fmt.contains("moreTriggerCharacter")) {
            for (auto &c : fmt["moreTriggerCharacter"]) {
              std::string s = c.get<std::string>();
              if (s.size() == 1)
                format_chars.push_back(s[0]);
            }
          }
          allow_formatting_on_type = true;
        } else if (fmt.is_boolean()) {
          allow_formatting_on_type = fmt.get<bool>();
        }
      }
      if (caps.contains("hoverProvider")) {
        auto &hover = caps["hoverProvider"];
        allow_hover =
            hover.is_boolean() ? hover.get<bool>() : hover.is_object();
      } else {
        allow_hover = false;
      }
      if (caps.contains("completionProvider")) {
        allow_completion = true;
        if (caps["completionProvider"].contains("resolveProvider"))
          allow_resolve =
              caps["completionProvider"]["resolveProvider"].get<bool>();
        if (caps["completionProvider"].contains("triggerCharacters")) {
          auto &chars = caps["completionProvider"]["triggerCharacters"];
          if (chars.is_array()) {
            for (auto &c : chars) {
              std::string str = c.get<std::string>();
              if (str.size() != 1)
                continue;
              trigger_chars.push_back(str[0]);
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
              end_chars.push_back(str[0]);
            }
          }
        }
      }
    }
    initialized = true;
    json initialized_message = {{"jsonrpc", "2.0"},
                                {"method", "initialized"},
                                {"params", json::object()}};
    send_raw(initialized_message);
  }
  ~LSPInstance() {
    for (auto &ed : editors)
      ed->lsp.store(nullptr);
    initialized = false;
    exited = true;
    if (pid == -1)
      return;
    json shutdown = {{"id", ++last_id}, {"method", "shutdown"}};
    send_raw(shutdown);
    pollfd pfd{stdout_fd, POLLIN, 0};
    poll(&pfd, 1, 500);
    json exit_msg = {{"method", "exit"}};
    send_raw(exit_msg);
    int waited = 0;
    while (waited < 100) {
      int status;
      pid_t res = waitpid(pid, &status, WNOHANG);
      if (res == pid)
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      waited += 10;
    }
    if (kill(pid, 0) == 0) {
      kill(pid, SIGKILL);
      waitpid(pid, nullptr, 0);
    }
    pid = -1;
    close(stdin_fd);
    close(stdout_fd);
  }
  bool init_process() {
    int in_pipe[2];
    int out_pipe[2];
    if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1) {
      perror("pipe");
      return false;
    }
    pid_t pid_tmp = fork();
    if (pid_tmp == -1) {
      perror("fork");
      return false;
    }
    if (pid_tmp == 0) {
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
      argv.push_back(const_cast<char *>(lsp_info->command.c_str()));
      for (auto &arg : lsp_info->args)
        argv.push_back(const_cast<char *>(arg.c_str()));
      argv.push_back(nullptr);
      execvp(lsp_info->command.c_str(), argv.data());
      perror("execvp");
      _exit(127);
    }
    pid = pid_tmp;
    stdin_fd = in_pipe[1];
    stdout_fd = out_pipe[0];
    close(in_pipe[0]);
    close(out_pipe[1]);
    return true;
  }
  void add(Editor *ed) {
    editors.push_back(ed);
    ed->lsp.store(this);
    char *buf = read(ed->root, 0, ed->root->char_count);
    std::string text(buf);
    free(buf);
    json message = {{"jsonrpc", "2.0"},
                    {"method", "textDocument/didOpen"},
                    {"params",
                     {{"textDocument",
                       {{"uri", ed->uri},
                        {"languageId", ed->lang.name},
                        {"version", 1},
                        {"text", text}}}}}};
    send_raw(message);
  }
  void remove(Editor *ed) {
    std::unique_lock lock(lsp::lsp_mutex);
    editors.erase(std::remove(editors.begin(), editors.end(), ed),
                  editors.end());
    lock.unlock();
    auto message = std::make_unique<LSPMessage>();
    message->message = {{"method", "textDocument/didClose"},
                        {"params", {{"textDocument", {{"uri", ed->uri}}}}}};
    send(std::move(message));
  }
  void work() {
    if (exited)
      return;
    int status;
    pid_t res = waitpid(pid, &status, WNOHANG);
    if (res == pid) {
      exited = true;
      pid = -1;
      return;
    }
    while (!outbox.empty()) {
      json message = outbox.front();
      std::string m = message.value("method", "");
      outbox.pop();
      send_raw(message);
    }
    pollfd pfd{stdout_fd, POLLIN | POLLHUP | POLLERR, 0};
    int r = poll(&pfd, 1, 0);
    if (r > 0 && pfd.revents & (POLLHUP | POLLERR)) {
      exited = true;
      pid = -1;
      return;
    }
    while ((r = poll(&pfd, 1, 0)) > 0) {
      if (pfd.revents & (POLLHUP | POLLERR)) {
        exited = true;
        pid = -1;
        return;
      }
      auto msg = read_lsp_message();
      if (!msg)
        break;
      if (msg->contains("id")) {
        uint32_t id = msg->at("id").get<uint32_t>();
        auto it = pending.find(id);
        if (it != pending.end()) {
          if (it->second->editor) {
            it->second->message = *msg;
            lsp_response_queue.push_back(std::move(it->second));
          } else {
            auto message = *std::move(it->second);
            message.message = *msg;
            message.callback(message);
          }
          pending.erase(it);
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
        Editor *ed = resolve_uri(uri);
        auto response = std::make_unique<LSPMessage>();
        response->editor = ed;
        response->message = *msg;
        response->callback = editor_handle_wrapper;
        if (ed)
          lsp_response_queue.push_back(std::move(response));
        else
          lsp_handle(*msg);
      }
    }
  }
  inline static void editor_handle_wrapper(const LSPMessage &message) {
    message.editor->lsp_handle(message.message);
  }
  void callbacks() {
    for (auto &message : lsp_response_queue)
      message->callback(*message);
    lsp_response_queue.clear();
  }
  inline void send_raw(const json &msg) {
    std::string payload = msg.dump();
    std::string header =
        "Content-Length: " + std::to_string(payload.size()) + "\r\n\r\n";
    std::string out = header + payload;
    const char *ptr = out.data();
    size_t remaining = out.size();
    while (remaining > 0) {
      ssize_t n = write(stdin_fd, ptr, remaining);
      if (n <= 0) {
        if (errno == EINTR)
          continue;
        break;
      }
      ptr += n;
      remaining -= n;
    }
  };
  inline std::optional<json> read_lsp_message() {
    std::string header;
    char c;
    while (true) {
      ssize_t n = read(stdout_fd, &c, 1);
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
      ssize_t n = read(stdout_fd, &body[got], len - got);
      if (n <= 0)
        return std::nullopt;
      got += n;
    }
    return json::parse(body);
  }
  inline Editor *resolve_uri(std::string uri) {
    if (uri.empty())
      return nullptr;
    for (auto &editor : editors)
      if (editor->uri == uri)
        return editor;
    return nullptr;
  }
  inline void lsp_handle(json &message) {
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
  void send(std::unique_ptr<LSPMessage> message) {
    if (pid == -1)
      return;
    message->message["jsonrpc"] = "2.0";
    if (message->callback)
      message->message["id"] = ++last_id;
    outbox.push(message->message);
    if (!message->callback)
      return;
    std::lock_guard lock(lsp::lsp_mutex);
    pending[last_id] = std::move(message);
  }
};

#endif

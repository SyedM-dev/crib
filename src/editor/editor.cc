#include "editor/editor.h"
#include "editor/decl.h"
#include "lsp/lsp.h"
#include "main.h"
#include "syntax/langs.h"
#include "utils/utils.h"

Editor::Editor(const char *filename_arg, uint8_t eol) {
  uint32_t len = 0;
  std::string filename = path_abs(filename_arg);
  this->unix_eol = eol & 1;
  char *str = load_file(filename.c_str(), &len, &this->unix_eol);
  if (!str) {
    str = (char *)malloc(1);
    *str = '\n';
    len = 1;
  }
  if ((eol >> 1) & 1)
    this->unix_eol = eol & 1;
  this->filename = filename;
  this->uri = path_to_file_uri(filename);
  this->cursor_preffered = UINT32_MAX;
  if (len == 0) {
    free(str);
    str = (char *)malloc(1);
    *str = '\n';
    len = 1;
  }
  this->scroll = {0, 0};
  this->cursor = {0, 0};
  this->size = {20, 20};
  this->root = load(str, len, optimal_chunk_size(len));
  free(str);
  this->lang = language_for_file(filename.c_str());
  if (parsers.find(this->lang.name) != parsers.end())
    this->parser = new Parser(this, this->lang.name, size.row + 5);
  if (this->lang.name == "css" || this->lang.name == "html" ||
      this->lang.name == "javascript" || this->lang.name == "markdown" ||
      this->lang.name == "typescript")
    this->is_css_color = true;
  if (len <= (1024 * 28)) {
    std::lock_guard lock(lsp::lsp_mutex);
    lsp::new_editors.push_back(this);
  }
  this->indents.compute_indent(this);
}

Editor::~Editor() {
  auto lsp = this->lsp.load();
  if (lsp)
    lsp->remove(this);
  if (this->parser)
    delete this->parser;
  this->parser = nullptr;
  free_rope(this->root);
}

void Editor::save() {
  if (!this->root)
    return;
  int version = this->lsp_version;
  uint32_t char_count = this->root->char_count;
  char *str = read(this->root, 0, char_count);
  if (!str)
    return;
  std::ofstream out(this->filename);
  if (!this->unix_eol) {
    for (uint32_t i = 0; i < char_count; ++i) {
      if (str[i] == '\n')
        out.put('\r');
      out.put(str[i]);
    }
  } else {
    out.write(str, char_count);
  }
  out.close();
  free(str);
  ui::bar.log("Written " + std::to_string(char_count) + " bytes to " +
              this->filename);
  auto lsp = this->lsp.load();
  if (lsp) {
    log("Saving %s", this->filename.c_str());
    auto message = std::make_unique<LSPMessage>();
    message->message = {{"method", "textDocument/didSave"},
                        {"params", {{"textDocument", {{"uri", this->uri}}}}}};
    lsp->send(std::move(message));
    if (lsp->allow_formatting) {
      log("Formatting %s", this->filename.c_str());
      json s_msg = {{"method", "textDocument/formatting"},
                    {"params",
                     {{"textDocument", {{"uri", this->uri}}},
                      {"options",
                       {{"tabSize", 2},
                        {"insertSpaces", true},
                        {"trimTrailingWhitespace", true},
                        {"trimFinalNewlines", true}}}}}};
      auto save_msg = std::make_unique<LSPMessage>();
      save_msg->editor = this;
      save_msg->message = s_msg;
      save_msg->callback = [s_msg, version](const LSPMessage &msg) {
        log("Formattin");
        if (version != msg.editor->lsp_version)
          return;
        auto &edits = msg.message["result"];
        if (edits.is_array()) {
          std::vector<TextEdit> t_edits;
          t_edits.reserve(edits.size());
          for (auto &edit : edits) {
            TextEdit t_edit;
            t_edit.text = edit.value("newText", "");
            t_edit.start.row = edit["range"]["start"]["line"];
            t_edit.start.col = edit["range"]["start"]["character"];
            t_edit.end.row = edit["range"]["end"]["line"];
            t_edit.end.col = edit["range"]["end"]["character"];
            msg.editor->utf8_normalize_edit(&t_edit);
            t_edits.push_back(t_edit);
          }
          msg.editor->apply_lsp_edits(t_edits, false);
          msg.editor->ensure_cursor();
          uint32_t char_count = msg.editor->root->char_count;
          char *str = read(msg.editor->root, 0, char_count);
          if (!str)
            return;
          std::ofstream out(msg.editor->filename);
          if (!msg.editor->unix_eol) {
            for (uint32_t i = 0; i < char_count; ++i) {
              if (str[i] == '\n')
                out.put('\r');
              out.put(str[i]);
            }
          } else {
            out.write(str, char_count);
          }
          out.close();
          free(str);
          auto save_msg = std::make_unique<LSPMessage>();
          save_msg->editor = msg.editor;
          save_msg->message = s_msg;
          save_msg->callback = [](const LSPMessage &) {};
          msg.editor->lsp.load()->send(std::move(save_msg));
        }
      };
      lsp->send(std::move(save_msg));
    }
  }
}

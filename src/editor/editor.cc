#include "editor/editor.h"
#include "editor/decl.h"
#include "lsp/lsp.h"
#include "main.h"
#include "syntax/langs.h"
#include "utils/utils.h"

Editor *new_editor(const char *filename_arg, Coord position, Coord size,
                   uint8_t eol) {
  Editor *editor = new Editor();
  if (!editor)
    return nullptr;
  uint32_t len = 0;
  std::string filename = path_abs(filename_arg);
  editor->unix_eol = eol & 1;
  char *str = load_file(filename.c_str(), &len, &editor->unix_eol);
  if (!str) {
    str = (char *)malloc(1);
    *str = '\n';
    len = 1;
  }
  if ((eol >> 1) & 1)
    editor->unix_eol = eol & 1;
  editor->filename = filename;
  editor->uri = path_to_file_uri(filename);
  editor->position = position;
  editor->size = size;
  editor->cursor_preffered = UINT32_MAX;
  if (len == 0) {
    free(str);
    str = (char *)malloc(1);
    *str = '\n';
    len = 1;
  }
  editor->root = load(str, len, optimal_chunk_size(len));
  free(str);
  editor->lang = language_for_file(filename.c_str());
  if (parsers.find(editor->lang.name) != parsers.end())
    editor->parser = new Parser(editor, editor->lang.name, size.row + 5);
  if (editor->lang.name == "css" || editor->lang.name == "html" ||
      editor->lang.name == "javascript" || editor->lang.name == "markdown" ||
      editor->lang.name == "typescript")
    editor->is_css_color = true;
  if (len <= (1024 * 28))
    request_add_to_lsp(editor->lang, editor);
  editor->indents.compute_indent(editor);
  return editor;
}

void free_editor(Editor *editor) {
  remove_from_lsp(editor);
  if (editor->parser)
    delete editor->parser;
  editor->parser = nullptr;
  free_rope(editor->root);
  delete editor;
}

void save_file(Editor *editor) {
  if (!editor || !editor->root)
    return;
  std::shared_lock lock(editor->knot_mtx);
  int version = editor->lsp_version;
  uint32_t char_count = editor->root->char_count;
  char *str = read(editor->root, 0, char_count);
  if (!str)
    return;
  lock.unlock();
  std::ofstream out(editor->filename);
  if (!editor->unix_eol) {
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
  bar.log("Written " + std::to_string(char_count) + " bytes to " +
          editor->filename);
  if (editor->lsp) {
    json save_msg = {{"jsonrpc", "2.0"},
                     {"method", "textDocument/didSave"},
                     {"params", {{"textDocument", {{"uri", editor->uri}}}}}};
    lsp_send(editor->lsp, save_msg, nullptr);
    if (editor->lsp->allow_formatting) {
      json msg = {{"jsonrpc", "2.0"},
                  {"method", "textDocument/formatting"},
                  {"params",
                   {{"textDocument", {{"uri", editor->uri}}},
                    {"options",
                     {{"tabSize", 2},
                      {"insertSpaces", true},
                      {"trimTrailingWhitespace", true},
                      {"trimFinalNewlines", true}}}}}};
      LSPPending *pending = new LSPPending();
      pending->editor = editor;
      pending->callback = [save_msg, version](Editor *editor,
                                              const json &message) {
        if (version != editor->lsp_version)
          return;
        auto &edits = message["result"];
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
            utf8_normalize_edit(editor, &t_edit);
            t_edits.push_back(t_edit);
          }
          apply_lsp_edits(editor, t_edits, false);
          ensure_scroll(editor);
          std::shared_lock lock(editor->knot_mtx);
          uint32_t char_count = editor->root->char_count;
          char *str = read(editor->root, 0, char_count);
          if (!str)
            return;
          lock.unlock();
          std::ofstream out(editor->filename);
          if (!editor->unix_eol) {
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
          lsp_send(editor->lsp, save_msg, nullptr);
        }
      };
      lsp_send(editor->lsp, msg, pending);
    }
  }
}

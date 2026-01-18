#include "editor/editor.h"
#include "editor/decl.h"
#include "lsp/lsp.h"
#include "utils/utils.h"
#include <shared_mutex>

Editor *new_editor(const char *filename_arg, Coord position, Coord size) {
  Editor *editor = new Editor();
  if (!editor)
    return nullptr;
  uint32_t len = 0;
  std::string filename = path_abs(filename_arg);
  char *str = load_file(filename.c_str(), &len);
  if (!str) {
    free_editor(editor);
    return nullptr;
  }
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
  if (editor->lang.name != "unknown")
    editor->parser = new Parser(editor->root, &editor->knot_mtx,
                                editor->lang.name, size.row + 5);
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
  char *str = read(editor->root, 0, editor->root->char_count);
  if (!str)
    return;
  lock.unlock();
  std::ofstream out(editor->filename);
  out.write(str, editor->root->char_count);
  out.close();
  free(str);
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
      pending->method = "textDocument/formatting";
      pending->callback = [save_msg, version](Editor *editor, std::string,
                                              json message) {
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
          char *str = read(editor->root, 0, editor->root->char_count);
          if (!str)
            return;
          lock.unlock();
          std::ofstream out(editor->filename);
          out.write(str, editor->root->char_count);
          free(str);
          lsp_send(editor->lsp, save_msg, nullptr);
        }
      };
      lsp_send(editor->lsp, msg, pending);
    }
  }
}

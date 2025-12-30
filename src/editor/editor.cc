#include "editor/editor.h"
#include "lsp/lsp.h"
#include "utils/utils.h"

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
  Language language = language_for_file(filename.c_str());
  if (language.name != "unknown" && len <= (1024 * 128)) {
    editor->ts.parser = ts_parser_new();
    editor->ts.language = language.fn();
    ts_parser_set_language(editor->ts.parser, editor->ts.language);
    editor->ts.query_file =
        get_exe_dir() + "/../grammar/" + language.name + ".scm";
  }
  if (len <= (1024 * 28))
    request_add_to_lsp(language, editor);
  return editor;
}

void free_tsset(TSSetMain *set) {
  if (set->parser)
    ts_parser_delete(set->parser);
  if (set->tree)
    ts_tree_delete(set->tree);
  if (set->query)
    ts_query_delete(set->query);
  for (auto &inj : set->injections) {
    if (inj.second.parser)
      ts_parser_delete(inj.second.parser);
    if (inj.second.query)
      ts_query_delete(inj.second.query);
    if (inj.second.tree)
      ts_tree_delete(inj.second.tree);
  }
}

void free_editor(Editor *editor) {
  remove_from_lsp(editor);
  free_tsset(&editor->ts);
  free_rope(editor->root);
  delete editor;
}

void save_file(Editor *editor) {
  if (!editor || !editor->root)
    return;
  char *str = read(editor->root, 0, editor->root->char_count);
  if (!str)
    return;
  std::ofstream out(editor->filename);
  out.write(str, editor->root->char_count);
  free(str);
  json msg = {{"jsonrpc", "2.0"},
              {"method", "textDocument/didSave"},
              {"params", {{"textDocument", {{"uri", editor->uri}}}}}};
  if (editor->lsp)
    lsp_send(editor->lsp, msg, nullptr);
}

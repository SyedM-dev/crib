#include "editor/editor.h"

void hover_diagnostic(Editor *editor) {
  std::shared_lock lock(editor->v_mtx);
  static uint32_t last_line = UINT32_MAX;
  if (last_line == editor->cursor.row && !editor->warnings_dirty)
    return;
  VWarn dummy;
  dummy.line = editor->cursor.row;
  editor->warnings_dirty = false;
  last_line = editor->cursor.row;
  auto first =
      std::lower_bound(editor->warnings.begin(), editor->warnings.end(), dummy);
  auto last =
      std::upper_bound(editor->warnings.begin(), editor->warnings.end(), dummy);
  std::vector<VWarn> warnings_at_line(first, last);
  if (warnings_at_line.size() == 0) {
    editor->diagnostics_active = false;
    return;
  }
  editor->diagnostics.clear();
  editor->diagnostics.warnings.swap(warnings_at_line);
  editor->diagnostics.render_first();
  editor->diagnostics_active = true;
}

void editor_worker(Editor *editor) {
  if (!editor || !editor->root)
    return;
  if (editor->parser)
    editor->parser->work();
  hover_diagnostic(editor);
  if (editor->completion.active && editor->completion.hover_dirty) {
    editor->completion.hover.render_first();
    editor->completion.hover_dirty = false;
  }
}

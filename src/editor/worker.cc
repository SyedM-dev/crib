#include "editor/editor.h"

// void hover_diagnostic(Editor *editor) {
//   static uint32_t last_line = UINT32_MAX;
//   if (last_line == editor->cursor.row && !editor->warnings_dirty)
//     return;
//   VWarn dummy;
//   dummy.line = editor->cursor.row;
//   editor->warnings_dirty = false;
//   last_line = editor->cursor.row;
//   auto first =
//       std::lower_bound(editor->warnings.begin(), editor->warnings.end(),
//       dummy);
//   auto last =
//       std::upper_bound(editor->warnings.begin(), editor->warnings.end(),
//       dummy);
//   std::vector<VWarn> warnings_at_line(first, last);
//   if (warnings_at_line.size() == 0) {
//     editor->diagnostics_active = false;
//     return;
//   }
//   editor->diagnostics.clear();
//   editor->diagnostics.warnings.swap(warnings_at_line);
//   editor->diagnostics.render_first();
//   editor->diagnostics_active = true;
// }

void Editor::work() {
  if (!this->root)
    return;
  if (this->parser)
    this->parser->work();
  // hover_diagnostic(this);
  // if (this->completion.active && this->completion.hover_dirty) {
  //   this->completion.hover.render_first();
  //   this->completion.hover_dirty = false;
  // }
}

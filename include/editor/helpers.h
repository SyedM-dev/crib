#ifndef EDITOR_HELPERS_H
#define EDITOR_HELPERS_H

#include "editor/editor.h"

void insert_str(Editor *editor, char *c, uint32_t len);
void insert_char(Editor *editor, char c);
void normal_mode(Editor *editor);
void backspace_edit(Editor *editor);
void delete_prev_word(Editor *editor);
void delete_next_word(Editor *editor);
void clear_hooks_at_line(Editor *editor, uint32_t line);
void cursor_prev_word(Editor *editor);
void cursor_next_word(Editor *editor);
void select_all(Editor *editor);
void fetch_lsp_hover(Editor *editor);
void handle_mouse(Editor *editor, KeyEvent event);
void indent_current_line(Editor *editor);
void dedent_current_line(Editor *editor);
void indent_selection(Editor *editor);
void dedent_selection(Editor *editor);
void paste(Editor *editor);
void copy(Editor *editor);
void cut(Editor *editor);

#endif

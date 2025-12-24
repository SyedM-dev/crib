extern "C" {
#include "../libs/libgrapheme/grapheme.h"
}
#include "../include/editor.h"
#include "../include/lsp.h"
#include "../include/main.h"
#include "../include/ts.h"
#include "../include/utils.h"

Editor *new_editor(const char *filename, Coord position, Coord size) {
  Editor *editor = new Editor();
  if (!editor)
    return nullptr;
  uint32_t len = 0;
  char *str = load_file(filename, &len);
  if (!str) {
    free_editor(editor);
    return nullptr;
  }
  editor->filename = filename;
  editor->uri = path_to_file_uri(filename);
  editor->position = position;
  editor->size = size;
  editor->cursor_preffered = UINT32_MAX;
  editor->root = load(str, len, optimal_chunk_size(len));
  free(str);
  if (len <= (1024 * 128)) {
    Language language = language_for_file(filename);
    editor->parser = ts_parser_new();
    editor->language = language.fn();
    ts_parser_set_language(editor->parser, editor->language);
    editor->query_file =
        get_exe_dir() + "/../grammar/" + language.name + ".scm";
    request_add_to_lsp(language, editor);
  }
  return editor;
}

void free_editor(Editor *editor) {
  remove_from_lsp(editor);
  ts_parser_delete(editor->parser);
  if (editor->tree)
    ts_tree_delete(editor->tree);
  if (editor->query)
    ts_query_delete(editor->query);
  free_rope(editor->root);
  delete editor;
}

void render_editor(Editor *editor) {
  uint32_t sel_start = 0, sel_end = 0;
  uint32_t numlen =
      EXTRA_META + static_cast<int>(std::log10(editor->root->line_count + 1));
  uint32_t render_width = editor->size.col - numlen;
  uint32_t render_x = editor->position.col + numlen;
  std::vector<std::pair<uint32_t, char>> v;
  for (size_t i = 0; i < 94; ++i)
    if (editor->hooks[i] != 0)
      v.push_back({editor->hooks[i], '!' + i});
  std::sort(v.begin(), v.end());
  auto hook_it = v.begin();
  while (hook_it != v.end() && hook_it->first <= editor->scroll.row)
    ++hook_it;

  // Iterators for hints and warnings (both already sorted)
  size_t hint_idx = 0;
  size_t warn_idx = 0;

  // Helper to advance hint iterator to current line
  auto advance_hints_to = [&](uint32_t row) {
    while (hint_idx < editor->hints.size() &&
           editor->hints[hint_idx].pos.row < row)
      ++hint_idx;
  };
  auto advance_warns_to = [&](uint32_t row) {
    while (warn_idx < editor->warnings.size() &&
           editor->warnings[warn_idx].line < row)
      ++warn_idx;
  };

  std::shared_lock knot_lock(editor->knot_mtx);
  if (editor->selection_active) {
    Coord start, end;
    if (editor->cursor >= editor->selection) {
      uint32_t prev_col, next_col;
      switch (editor->selection_type) {
      case CHAR:
        start = editor->selection;
        end = move_right(editor, editor->cursor, 1);
        break;
      case WORD:
        word_boundaries(editor, editor->selection, &prev_col, &next_col,
                        nullptr, nullptr);
        start = {editor->selection.row, prev_col};
        end = editor->cursor;
        break;
      case LINE:
        start = {editor->selection.row, 0};
        end = editor->cursor;
        break;
      }
    } else {
      start = editor->cursor;
      uint32_t prev_col, next_col, line_len;
      switch (editor->selection_type) {
      case CHAR:
        end = move_right(editor, editor->selection, 1);
        break;
      case WORD:
        word_boundaries(editor, editor->selection, &prev_col, &next_col,
                        nullptr, nullptr);
        end = {editor->selection.row, next_col};
        break;
      case LINE:
        LineIterator *it = begin_l_iter(editor->root, editor->selection.row);
        char *line = next_line(it, &line_len);
        if (!line)
          return;
        if (line_len > 0 && line[line_len - 1] == '\n')
          line_len--;
        free(it->buffer);
        free(it);
        end = {editor->selection.row, line_len};
        break;
      }
    }
    sel_start = line_to_byte(editor->root, start.row, nullptr) + start.col;
    sel_end = line_to_byte(editor->root, end.row, nullptr) + end.col;
  }

  // Helper for warning colors based on type
  auto warn_colors = [](int8_t type) -> std::pair<uint32_t, uint32_t> {
    switch (type) {
    case 1: // info
      return {0x7fbfff, 0};
    case 2: // warn
      return {0xffd166, 0};
    case 3: // error
      return {0xff5f5f, 0};
    default: // neutral
      return {0xaaaaaa, 0};
    }
  };

  // Helper to get nth line (0-based) from VAI text (ASCII/UTF-8)
  auto ai_line_span = [&](const VAI &ai,
                          uint32_t n) -> std::pair<const char *, uint32_t> {
    const char *p = ai.text;
    uint32_t line_no = 0;
    const char *start = p;
    uint32_t len = 0;
    for (uint32_t i = 0; i < ai.len; i++) {
      if (ai.text[i] == '\n') {
        if (line_no == n) {
          len = i - (start - ai.text);
          return {start, len};
        }
        line_no++;
        start = ai.text + i + 1;
      }
    }
    // last line (no trailing newline)
    if (line_no == n) {
      len = ai.text + ai.len - start;
      return {start, len};
    }
    return {nullptr, 0};
  };

  Coord cursor = {UINT32_MAX, UINT32_MAX};
  uint32_t line_index = editor->scroll.row;
  SpanCursor span_cursor(editor->spans);
  SpanCursor def_span_cursor(editor->def_spans);
  LineIterator *it = begin_l_iter(editor->root, line_index);
  if (!it)
    return;
  uint32_t rendered_rows = 0;
  uint32_t global_byte_offset = line_to_byte(editor->root, line_index, nullptr);
  span_cursor.sync(global_byte_offset);
  def_span_cursor.sync(global_byte_offset);

  const bool ai_active = editor->ai.text && editor->ai.len > 0;
  const uint32_t ai_row = ai_active ? editor->ai.pos.row : UINT32_MAX;
  const uint32_t ai_lines = ai_active ? editor->ai.lines : 0;

  while (rendered_rows < editor->size.row) {
    advance_hints_to(line_index);
    advance_warns_to(line_index);

    const Fold *fold = fold_for_line(editor->folds, line_index);
    if (fold) {
      update(editor->position.row + rendered_rows, editor->position.col, "",
             0xAAAAAA, 0, 0);
      char buf[16];
      int len = snprintf(buf, sizeof(buf), "%*u ", numlen - 3, fold->start + 1);
      uint32_t num_color =
          editor->cursor.row == fold->start ? 0xFFFFFF : 0x555555;
      for (int i = 0; i < len; i++)
        update(editor->position.row + rendered_rows,
               editor->position.col + i + 2, (char[2]){buf[i], 0}, num_color, 0,
               0);
      const char marker[15] = "... folded ...";
      uint32_t i = 0;
      for (; i < 14 && i < render_width; i++)
        update(rendered_rows, i + render_x, (char[2]){marker[i], 0}, 0xc6c6c6,
               0, 0);
      for (; i < render_width; i++)
        update(rendered_rows, i + render_x, " ", 0xc6c6c6, 0, 0);
      rendered_rows++;

      uint32_t skip_until = fold->end;
      while (line_index <= skip_until) {
        if (hook_it != v.end() && hook_it->first == line_index + 1)
          hook_it++;
        if (hint_idx < editor->hints.size() &&
            editor->hints[hint_idx].pos.row == line_index)
          hint_idx++;
        if (warn_idx < editor->warnings.size() &&
            editor->warnings[warn_idx].line == line_index)
          warn_idx++;
        uint32_t line_len;
        char *line = next_line(it, &line_len);
        if (!line)
          break;
        global_byte_offset += line_len;
        if (line_len > 0 && line[line_len - 1] == '\n')
          global_byte_offset--;
        global_byte_offset++;
        line_index++;
      }
      continue;
    }
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (!line)
      break;
    if (line_len > 0 && line[line_len - 1] == '\n')
      line_len--;
    uint32_t current_byte_offset = 0;
    if (rendered_rows == 0)
      current_byte_offset += editor->scroll.col;

    // AI handling: determine if this line is overridden by AI
    bool ai_this_line =
        ai_active && line_index >= ai_row && line_index <= ai_row + ai_lines;
    bool ai_first_line = ai_this_line && line_index == ai_row;

    while ((ai_this_line ? current_byte_offset <= line_len
                         : current_byte_offset < line_len) &&
           rendered_rows < editor->size.row) {
      uint32_t color = editor->cursor.row == line_index ? 0x222222 : 0;
      if (current_byte_offset == 0 || rendered_rows == 0) {
        const char *hook = nullptr;
        char h[2] = {0, 0};
        if (hook_it != v.end() && hook_it->first == line_index + 1) {
          h[0] = hook_it->second;
          hook = h;
          hook_it++;
        }
        update(editor->position.row + rendered_rows, editor->position.col, hook,
               0xAAAAAA, 0, 0);
        char buf[16];
        int len =
            snprintf(buf, sizeof(buf), "%*u ", numlen - 3, line_index + 1);
        uint32_t num_color =
            editor->cursor.row == line_index ? 0xFFFFFF : 0x555555;
        for (int i = 0; i < len; i++)
          update(editor->position.row + rendered_rows,
                 editor->position.col + i + 2, (char[2]){buf[i], 0}, num_color,
                 0, 0);
      } else {
        for (uint32_t i = 0; i < numlen; i++)
          update(editor->position.row + rendered_rows, editor->position.col + i,
                 " ", 0, 0, 0);
      }
      uint32_t col = 0;
      uint32_t local_render_offset = 0;
      uint32_t line_left = line_len - current_byte_offset;

      // For AI extra lines (line > ai_row), we don't render real text
      if (ai_this_line && !ai_first_line) {
        const uint32_t ai_line_no = line_index - ai_row;
        auto [aptr, alen] = ai_line_span(editor->ai, ai_line_no);
        if (aptr && alen) {
          uint32_t draw = std::min<uint32_t>(alen, render_width);
          update(editor->position.row + rendered_rows, render_x,
                 std::string(aptr, draw).c_str(), 0x666666, 0, CF_ITALIC);
          col = draw;
        }
        while (col < render_width) {
          update(editor->position.row + rendered_rows, render_x + col, " ", 0,
                 0 | color, 0);
          col++;
        }
        rendered_rows++;
        break; // move to next screen row
      }

      while (line_left > 0 && col < render_width) {
        // Render pending hints at this byte offset
        while (hint_idx < editor->hints.size() &&
               editor->hints[hint_idx].pos.row == line_index &&
               editor->hints[hint_idx].pos.col ==
                   current_byte_offset + local_render_offset) {
          const VHint &vh = editor->hints[hint_idx];
          uint32_t draw = std::min<uint32_t>(vh.len, render_width - col);
          if (draw == 0)
            break;
          update(editor->position.row + rendered_rows, render_x + col,
                 std::string(vh.text, draw).c_str(), 0x777777, 0 | color,
                 CF_ITALIC);
          col += draw;
          ++hint_idx;
          if (col >= render_width)
            break;
        }
        if (col >= render_width)
          break;

        // AI first line: stop underlying text at ai.pos.col, then render AI,
        // clip
        if (ai_first_line &&
            (current_byte_offset + local_render_offset) >= editor->ai.pos.col) {
          // render AI first line
          auto [aptr, alen] = ai_line_span(editor->ai, 0);
          if (aptr && alen) {
            uint32_t draw = std::min<uint32_t>(alen, render_width - col);
            update(editor->position.row + rendered_rows, render_x + col,
                   std::string(aptr, draw).c_str(), 0x666666, 0 | color,
                   CF_ITALIC);
            col += draw;
          }
          // fill rest and break
          while (col < render_width) {
            update(editor->position.row + rendered_rows, render_x + col, " ", 0,
                   0 | color, 0);
            col++;
          }
          rendered_rows++;
          current_byte_offset = line_len; // hide rest of real text
          goto after_line_body;
        }

        if (line_index == editor->cursor.row &&
            editor->cursor.col == (current_byte_offset + local_render_offset)) {
          cursor.row = editor->position.row + rendered_rows;
          cursor.col = render_x + col;
        }
        uint32_t absolute_byte_pos =
            global_byte_offset + current_byte_offset + local_render_offset;
        Highlight *hl = span_cursor.get_highlight(absolute_byte_pos);
        Highlight *def_hl = def_span_cursor.get_highlight(absolute_byte_pos);
        uint32_t fg = hl ? hl->fg : 0xFFFFFF;
        uint32_t bg = hl ? hl->bg : 0;
        uint8_t fl = hl ? hl->flags : 0;
        if (def_hl) {
          if (def_hl->fg != 0)
            fg = def_hl->fg;
          if (def_hl->bg != 0)
            bg = def_hl->bg;
          fl |= def_hl->flags;
        }
        if (editor->selection_active && absolute_byte_pos >= sel_start &&
            absolute_byte_pos < sel_end)
          bg = 0x555555;
        uint32_t cluster_len = grapheme_next_character_break_utf8(
            line + current_byte_offset + local_render_offset, line_left);
        std::string cluster(line + current_byte_offset + local_render_offset,
                            cluster_len);
        int width = display_width(cluster.c_str(), cluster_len);
        if (col + width > render_width)
          break;
        update(editor->position.row + rendered_rows, render_x + col,
               cluster.c_str(), fg, bg | color, fl);
        local_render_offset += cluster_len;
        line_left -= cluster_len;
        col += width;
        while (width-- > 1)
          update(editor->position.row + rendered_rows, render_x + col - width,
                 "\x1b", fg, bg | color, fl);
      }

      if (line_index == editor->cursor.row &&
          editor->cursor.col == (current_byte_offset + local_render_offset)) {
        cursor.row = editor->position.row + rendered_rows;
        cursor.col = render_x + col;
      }

      // Trailing selection block
      if (editor->selection_active &&
          global_byte_offset + line_len + 1 > sel_start &&
          global_byte_offset + line_len + 1 <= sel_end && col < render_width) {
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               0x555555 | color, 0);
        col++;
      }

      // Render warning text at end (does not affect wrapping)
      if (warn_idx < editor->warnings.size() &&
          editor->warnings[warn_idx].line == line_index && col < render_width) {
        const VWarn &w = editor->warnings[warn_idx];
        auto [wfg, wbg] = warn_colors(w.type);
        uint32_t draw = std::min<uint32_t>(w.len, render_width - col);
        if (draw)
          update(editor->position.row + rendered_rows, render_x + col,
                 std::string(w.text, draw).c_str(), wfg, wbg | color,
                 CF_ITALIC);
        // do not advance col for padding skip; we still fill remaining spaces
      }

      while (col < render_width) {
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               0 | color, 0);
        col++;
      }
      rendered_rows++;
      current_byte_offset += local_render_offset;

    after_line_body:
      break; // proceed to next screen row
    }

    if (line_len == 0 ||
        (current_byte_offset >= line_len && rendered_rows == 0)) {
      uint32_t color = editor->cursor.row == line_index ? 0x222222 : 0;
      const char *hook = nullptr;
      char h[2] = {0, 0};
      if (hook_it != v.end() && hook_it->first == line_index + 1) {
        h[0] = hook_it->second;
        hook = h;
        hook_it++;
      }
      update(editor->position.row + rendered_rows, editor->position.col, hook,
             0xAAAAAA, 0, 0);
      char buf[16];
      int len = snprintf(buf, sizeof(buf), "%*u ", numlen - 3, line_index + 1);
      uint32_t num_color =
          editor->cursor.row == line_index ? 0xFFFFFF : 0x555555;
      for (int i = 0; i < len; i++)
        update(editor->position.row + rendered_rows,
               editor->position.col + i + 2, (char[2]){buf[i], 0}, num_color, 0,
               0);
      if (editor->cursor.row == line_index) {
        cursor.row = editor->position.row + rendered_rows;
        cursor.col = render_x;
      }
      uint32_t col = 0;
      if (editor->selection_active &&
          global_byte_offset + line_len + 1 > sel_start &&
          global_byte_offset + line_len + 1 <= sel_end) {
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               0x555555 | color, 0);
        col++;
      }
      // warning on empty line
      if (warn_idx < editor->warnings.size() &&
          editor->warnings[warn_idx].line == line_index && col < render_width) {
        const VWarn &w = editor->warnings[warn_idx];
        auto [wfg, wbg] = warn_colors(w.type);
        uint32_t draw = std::min<uint32_t>(w.len, render_width - col);
        if (draw)
          update(editor->position.row + rendered_rows, render_x + col,
                 std::string(w.text, draw).c_str(), wfg, wbg | color,
                 CF_ITALIC);
      }
      while (col < render_width) {
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               0 | color, 0);
        col++;
      }
      rendered_rows++;
    }
    global_byte_offset += line_len + 1;
    line_index++;
  }
  if (cursor.row != UINT32_MAX && cursor.col != UINT32_MAX) {
    int type = 0;
    switch (mode) {
    case NORMAL:
      type = BLOCK;
      break;
    case INSERT:
      type = CURSOR;
      break;
    case JUMPER:
    case SELECT:
      type = UNDERLINE;
      break;
    }
    set_cursor(cursor.row, cursor.col, type, true);
  }
  while (rendered_rows < editor->size.row) {
    for (uint32_t col = 0; col < editor->size.col; col++)
      update(editor->position.row + rendered_rows, editor->position.col + col,
             " ", 0xFFFFFF, 0, 0);
    rendered_rows++;
  }
  free(it->buffer);
  free(it);
}

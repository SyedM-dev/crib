#include "editor/editor.h"
#include "editor/folds.h"
#include "main.h"

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
  std::unique_lock warn_lock(editor->v_mtx);
  auto warn_it = editor->warnings.begin();
  while (warn_it != editor->warnings.end() &&
         warn_it->line < editor->scroll.row)
    ++warn_it;
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
  while (rendered_rows < editor->size.row) {
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
        while (warn_it != editor->warnings.end() && warn_it->line == line_index)
          ++warn_it;
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
    uint32_t content_end = line_len;
    while (content_end > 0 &&
           (line[content_end - 1] == ' ' || line[content_end - 1] == '\t'))
      content_end--;
    uint32_t content_start = 0;
    while (content_start < line_len &&
           (line[content_start] == ' ' || line[content_start] == '\t'))
      content_start++;
    std::vector<VWarn> line_warnings;
    while (warn_it != editor->warnings.end() && warn_it->line == line_index) {
      line_warnings.push_back(*warn_it);
      ++warn_it;
    }
    uint32_t current_byte_offset = 0;
    if (rendered_rows == 0)
      current_byte_offset += editor->scroll.col;
    while (current_byte_offset < line_len && rendered_rows < editor->size.row) {
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
      while (line_left > 0 && col < render_width) {
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
        uint32_t u_color = 0;
        for (const auto &w : line_warnings) {
          if (w.start <= current_byte_offset + local_render_offset &&
              current_byte_offset + local_render_offset < w.end) {
            switch (w.type) {
            case 1:
              u_color = 0xff0000;
              fl |= CF_UNDERLINE;
              break;
            case 2:
              u_color = 0xffff00;
              fl |= CF_UNDERLINE;
              break;
            case 3:
              u_color = 0xff00ff;
              fl |= CF_UNDERLINE;
              break;
            case 4:
              u_color = 0xA0A0A0;
              fl |= CF_UNDERLINE;
              break;
            }
          }
        }
        uint32_t cluster_len = grapheme_next_character_break_utf8(
            line + current_byte_offset + local_render_offset, line_left);
        std::string cluster(line + current_byte_offset + local_render_offset,
                            cluster_len);
        int width = display_width(cluster.c_str(), cluster_len);
        if (col + width > render_width)
          break;
        if (current_byte_offset + local_render_offset >= content_start &&
            current_byte_offset + local_render_offset < content_end) {
          update(editor->position.row + rendered_rows, render_x + col,
                 cluster.c_str(), fg, bg | color, fl, u_color);
        } else {
          if (cluster[0] == ' ') {
            update(editor->position.row + rendered_rows, render_x + col, "·",
                   0x282828, bg | color, fl, u_color);
          } else {
            update(editor->position.row + rendered_rows, render_x + col, "->  ",
                   0x282828, bg | color, (fl & ~CF_BOLD) | CF_ITALIC, u_color);
          }
        }
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
      if (editor->selection_active &&
          global_byte_offset + line_len + 1 > sel_start &&
          global_byte_offset + line_len + 1 <= sel_end && col < render_width) {
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               0x555555 | color, 0);
        col++;
      }
      if (!line_warnings.empty() && line_left == 0) {
        VWarn warn = line_warnings.front();
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               color, 0);
        col++;
        for (size_t i = 0; i < line_warnings.size(); i++) {
          if (line_warnings[i].type < warn.type)
            warn = line_warnings[i];
          std::string err_sym = " ";
          uint32_t fg_color = 0;
          switch (line_warnings[i].type) {
          case 1:
            err_sym = "";
            fg_color = 0xFF0000;
            goto final;
          case 2:
            err_sym = "";
            fg_color = 0xFFFF00;
            goto final;
          case 3:
            err_sym = "";
            fg_color = 0xFF00FF;
            goto final;
          case 4:
            err_sym = "";
            fg_color = 0xAAAAAA;
            goto final;
          final:
            if (col < render_width) {
              update(editor->position.row + rendered_rows, render_x + col,
                     err_sym, fg_color, color, 0);
              col++;
              update(editor->position.row + rendered_rows, render_x + col, " ",
                     fg_color, color, 0);
              col++;
            }
          }
        }
        if (col < render_width) {
          update(editor->position.row + rendered_rows, render_x + col, " ", 0,
                 0 | color, 0);
          col++;
        }
        size_t warn_idx = 0;
        uint32_t fg_color = 0;
        switch (warn.type) {
        case 1:
          fg_color = 0xFF0000;
          break;
        case 2:
          fg_color = 0xFFFF00;
          break;
        case 3:
          fg_color = 0xFF00FF;
          break;
        case 4:
          fg_color = 0xAAAAAA;
          break;
        }
        while (col < render_width && warn_idx < warn.text.length()) {
          uint32_t cluster_len = grapheme_next_character_break_utf8(
              warn.text.c_str() + warn_idx, warn.text.length() - warn_idx);
          std::string cluster = warn.text.substr(warn_idx, cluster_len);
          int width = display_width(cluster.c_str(), cluster_len);
          if (col + width > render_width)
            break;
          update(editor->position.row + rendered_rows, render_x + col,
                 cluster.c_str(), fg_color, color, 0);
          col += width;
          warn_idx += cluster_len;
          while (width-- > 1)
            update(editor->position.row + rendered_rows, render_x + col - width,
                   "\x1b", fg_color, color, 0);
        }
        line_warnings.clear();
      }
      while (col < render_width) {
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               0 | color, 0);
        col++;
      }
      rendered_rows++;
      current_byte_offset += local_render_offset;
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
      if (!line_warnings.empty()) {
        VWarn warn = line_warnings.front();
        update(editor->position.row + rendered_rows, render_x + col, " ", 0,
               color, 0);
        col++;
        for (size_t i = 0; i < line_warnings.size(); i++) {
          if (line_warnings[i].type < warn.type)
            warn = line_warnings[i];
          std::string err_sym = " ";
          uint32_t fg_color = 0;
          switch (line_warnings[i].type) {
          case 1:
            err_sym = "";
            fg_color = 0xFF0000;
            goto final2;
          case 2:
            err_sym = "";
            fg_color = 0xFFFF00;
            goto final2;
          case 3:
            err_sym = "";
            fg_color = 0xFF00FF;
            goto final2;
          case 4:
            err_sym = "";
            fg_color = 0xAAAAAA;
            goto final2;
          final2:
            if (col < render_width) {
              update(editor->position.row + rendered_rows, render_x + col,
                     err_sym, fg_color, color, 0);
              col++;
              update(editor->position.row + rendered_rows, render_x + col, " ",
                     fg_color, color, 0);
              col++;
            }
          }
        }
        if (col < render_width) {
          update(editor->position.row + rendered_rows, render_x + col, " ", 0,
                 0 | color, 0);
          col++;
        }
        size_t warn_idx = 0;
        uint32_t fg_color = 0;
        switch (warn.type) {
        case 1:
          fg_color = 0xFF0000;
          break;
        case 2:
          fg_color = 0xFFFF00;
          break;
        case 3:
          fg_color = 0xFF00FF;
          break;
        case 4:
          fg_color = 0xAAAAAA;
          break;
        }
        while (col < render_width && warn_idx < warn.text.length()) {
          uint32_t cluster_len = grapheme_next_character_break_utf8(
              warn.text.c_str() + warn_idx, warn.text.length() - warn_idx);
          std::string cluster = warn.text.substr(warn_idx, cluster_len);
          int width = display_width(cluster.c_str(), cluster_len);
          if (col + width > render_width)
            break;
          update(editor->position.row + rendered_rows, render_x + col,
                 cluster.c_str(), fg_color, color, 0);
          col += width;
          warn_idx += cluster_len;
          while (width-- > 1)
            update(editor->position.row + rendered_rows, render_x + col - width,
                   "\x1b", fg_color, color, 0);
        }
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
    if (editor->hover_active)
      editor->hover.render(cursor);
    else if (editor->diagnostics_active)
      editor->diagnostics.render(cursor);
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

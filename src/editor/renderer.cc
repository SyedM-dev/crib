#include "editor/editor.h"
#include "io/sysio.h"
#include "main.h"
#include "syntax/decl.h"
#include "syntax/parser.h"

void Editor::render(std::vector<ScreenCell> &buffer, Coord size, Coord pos) {
  this->size = size;
  uint32_t sel_start = 0, sel_end = 0;
  uint32_t numlen =
      EXTRA_META + static_cast<int>(std::log10(this->root->line_count + 1));
  uint32_t render_width = size.col - numlen;
  uint32_t render_x = pos.col + numlen + 1;
  std::vector<std::pair<uint32_t, char>> v;
  for (size_t i = 0; i < 94; ++i)
    if (this->hooks[i] != 0)
      v.push_back({this->hooks[i], '!' + i});
  std::sort(v.begin(), v.end());
  auto hook_it = v.begin();
  while (hook_it != v.end() && hook_it->first <= this->scroll.row)
    ++hook_it;
  auto warn_it = this->warnings.begin();
  while (warn_it != this->warnings.end() && warn_it->line < this->scroll.row)
    ++warn_it;
  LineData *line_data = nullptr;
  auto get_type = [&](uint32_t col) {
    if (!line_data)
      return 0;
    for (auto const &token : line_data->tokens)
      if (token.start <= col && token.end > col)
        return (int)token.type;
    return 0;
  };
  Coord screen = get_size();
  auto update = [&](uint32_t row, uint32_t col, std::string text, uint32_t fg,
                    uint32_t bg, uint8_t flags, uint32_t u_color,
                    uint32_t width) {
    if (row >= screen.row || col >= screen.col)
      return;
    ScreenCell &c = buffer[row * screen.col + col];
    c.utf8 = text;
    c.width = width;
    c.fg = fg;
    c.bg = bg;
    c.flags = flags;
    c.ul_color = u_color;
  };
  if (this->selection_active) {
    Coord start, end;
    if (this->cursor >= this->selection) {
      uint32_t prev_col, next_col;
      switch (this->selection_type) {
      case CHAR:
        start = this->selection;
        end = this->move_right(this->cursor, 1);
        break;
      case WORD:
        this->word_boundaries(this->selection, &prev_col, &next_col, nullptr,
                              nullptr);
        start = {this->selection.row, prev_col};
        end = this->cursor;
        break;
      case LINE:
        start = {this->selection.row, 0};
        end = this->cursor;
        break;
      }
    } else {
      start = this->cursor;
      uint32_t prev_col, next_col, line_len;
      switch (this->selection_type) {
      case CHAR:
        end = this->move_right(this->selection, 1);
        break;
      case WORD:
        this->word_boundaries(this->selection, &prev_col, &next_col, nullptr,
                              nullptr);
        end = {this->selection.row, next_col};
        break;
      case LINE:
        LineIterator *it = begin_l_iter(this->root, this->selection.row);
        char *line = next_line(it, &line_len);
        if (!line)
          return;
        if (line_len > 0 && line[line_len - 1] == '\n')
          line_len--;
        free(it->buffer);
        free(it);
        end = {this->selection.row, line_len};
        break;
      }
    }
    sel_start = line_to_byte(this->root, start.row, nullptr) + start.col;
    sel_end = line_to_byte(this->root, end.row, nullptr) + end.col;
  }
  Coord cursor = {UINT32_MAX, UINT32_MAX};
  uint32_t line_index = this->scroll.row;
  LineIterator *it = begin_l_iter(this->root, line_index);
  if (!it)
    return;
  uint32_t prev_col, next_col;
  std::string word;
  this->word_boundaries_exclusive(this->cursor, &prev_col, &next_col);
  if (next_col - prev_col > 0 && next_col - prev_col < 256 - 4) {
    uint32_t offset = line_to_byte(this->root, this->cursor.row, nullptr);
    char *word_ptr = read(this->root, offset + prev_col, next_col - prev_col);
    if (word_ptr) {
      word = std::string(word_ptr, next_col - prev_col);
      free(word_ptr);
    }
  }
  this->extra_hl.render(this->root, line_index, word, this->is_css_color);
  uint32_t rendered_rows = 0;
  uint32_t global_byte_offset = line_to_byte(this->root, line_index, nullptr);
  while (rendered_rows < this->size.row) {
    uint32_t line_len;
    char *line = next_line(it, &line_len);
    if (this->parser) {
      if (line_data)
        line_data = this->parser->line_tree.next();
      else
        line_data = this->parser->line_tree.start_iter(line_index);
    }
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
    while (warn_it != this->warnings.end() && warn_it->line == line_index) {
      line_warnings.push_back(*warn_it);
      ++warn_it;
    }
    uint32_t current_byte_offset = 0;
    if (rendered_rows == 0)
      current_byte_offset += this->scroll.col;
    while (current_byte_offset < line_len && rendered_rows < this->size.row) {
      uint32_t color = this->cursor.row == line_index ? 0x222222 : 0;
      if (current_byte_offset == 0 || rendered_rows == 0) {
        const char *hook = "";
        char h[2] = {0, 0};
        if (hook_it != v.end() && hook_it->first == line_index + 1) {
          h[0] = hook_it->second;
          hook = h;
          hook_it++;
        }
        update(pos.row + rendered_rows, pos.col, hook, 0xAAAAAA, 0, 0, 0, 1);
        char buf[16];
        int len = snprintf(buf, sizeof(buf), "%*u ", numlen, line_index + 1);
        uint32_t num_color =
            this->cursor.row == line_index ? 0xFFFFFF : 0x555555;
        for (int i = 0; i < len; i++)
          update(pos.row + rendered_rows, pos.col + i, (char[2]){buf[i], 0},
                 num_color, 0, 0, 0, 1);
      } else {
        for (uint32_t i = 0; i < numlen + 1; i++)
          update(pos.row + rendered_rows, pos.col + i, " ", 0, 0, 0, 0, 1);
      }
      uint32_t col = 0;
      uint32_t local_render_offset = 0;
      uint32_t line_left = line_len - current_byte_offset;
      while (line_left > 0 && col < render_width) {
        if (line_index == this->cursor.row &&
            this->cursor.col == (current_byte_offset + local_render_offset)) {
          cursor.row = pos.row + rendered_rows;
          cursor.col = render_x + col;
        }
        uint32_t absolute_byte_pos =
            global_byte_offset + current_byte_offset + local_render_offset;
        const Highlight *hl = nullptr;
        if (this->parser)
          hl = &highlights[get_type(current_byte_offset + local_render_offset)];
        std::optional<std::pair<uint32_t, uint32_t>> extra = this->extra_hl.get(
            {line_index, current_byte_offset + local_render_offset});
        uint32_t fg = extra && extra->second != UINT32_MAX
                          ? extra->first
                          : (hl ? hl->fg : 0xFFFFFF);
        uint32_t bg = extra && extra->second != UINT32_MAX
                          ? extra->second
                          : (hl ? hl->bg : 0x000000);
        uint8_t fl =
            (hl ? hl->flags : 0) |
            (extra ? (extra->second != UINT32_MAX ? CF_BOLD : CF_UNDERLINE)
                   : 0);
        if (this->selection_active && absolute_byte_pos >= sel_start &&
            absolute_byte_pos < sel_end)
          bg = bg | 0x555555;
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
          update(pos.row + rendered_rows, render_x + col, cluster.c_str(), fg,
                 bg | color, fl, u_color, width);
        } else {
          if (cluster[0] == ' ') {
            update(pos.row + rendered_rows, render_x + col, "·", 0x282828,
                   bg | color, fl, u_color, 1);
          } else {
            update(pos.row + rendered_rows, render_x + col, "->  ", 0x282828,
                   bg | color, (fl & ~CF_BOLD) | CF_ITALIC, u_color, 4);
          }
        }
        local_render_offset += cluster_len;
        line_left -= cluster_len;
        col += width;
        while (width-- > 1)
          update(pos.row + rendered_rows, render_x + col - width, "\x1b", fg,
                 bg | color, fl, u_color, 0);
      }
      if (line_index == this->cursor.row &&
          this->cursor.col == (current_byte_offset + local_render_offset)) {
        cursor.row = pos.row + rendered_rows;
        cursor.col = render_x + col;
      }
      if (this->selection_active &&
          global_byte_offset + line_len + 1 > sel_start &&
          global_byte_offset + line_len + 1 <= sel_end && col < render_width) {
        update(pos.row + rendered_rows, render_x + col, " ", 0,
               0x555555 | color, 0, 0, 1);
        col++;
      }
      if (!line_warnings.empty() && line_left == 0) {
        VWarn warn = line_warnings.front();
        update(pos.row + rendered_rows, render_x + col, " ", 0, color, 0, 0, 1);
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
              update(pos.row + rendered_rows, render_x + col, err_sym, fg_color,
                     color, 0, 0, 1);
              col++;
              update(pos.row + rendered_rows, render_x + col, " ", fg_color,
                     color, 0, 0, 1);
              col++;
            }
          }
        }
        if (col < render_width) {
          update(pos.row + rendered_rows, render_x + col, " ", 0, 0 | color, 0,
                 0, 1);
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
          update(pos.row + rendered_rows, render_x + col, cluster.c_str(),
                 fg_color, color, 0, 0, width);
          col += width;
          warn_idx += cluster_len;
          while (width-- > 1)
            update(pos.row + rendered_rows, render_x + col - width, "\x1b",
                   fg_color, color, 0, 0, 0);
        }
        line_warnings.clear();
      }
      while (col < render_width) {
        update(pos.row + rendered_rows, render_x + col, " ", 0, 0 | color, 0, 0,
               1);
        col++;
      }
      rendered_rows++;
      current_byte_offset += local_render_offset;
    }
    if (line_len == 0 ||
        (current_byte_offset >= line_len && rendered_rows == 0)) {
      uint32_t color = this->cursor.row == line_index ? 0x222222 : 0;
      const char *hook = "";
      char h[2] = {0, 0};
      if (hook_it != v.end() && hook_it->first == line_index + 1) {
        h[0] = hook_it->second;
        hook = h;
        hook_it++;
      }
      update(pos.row + rendered_rows, pos.col, hook, 0xAAAAAA, 0, 0, 0, 1);
      char buf[16];
      int len = snprintf(buf, sizeof(buf), "%*u ", numlen, line_index + 1);
      uint32_t num_color = this->cursor.row == line_index ? 0xFFFFFF : 0x555555;
      for (int i = 0; i < len; i++)
        update(pos.row + rendered_rows, pos.col + i, (char[2]){buf[i], 0},
               num_color, 0, 0, 0, 1);
      if (this->cursor.row == line_index) {
        cursor.row = pos.row + rendered_rows;
        cursor.col = render_x;
      }
      uint32_t col = 0;
      if (this->selection_active &&
          global_byte_offset + line_len + 1 > sel_start &&
          global_byte_offset + line_len + 1 <= sel_end) {
        update(pos.row + rendered_rows, render_x + col, " ", 0,
               0x555555 | color, 0, 0, 1);
        col++;
      }
      if (!line_warnings.empty()) {
        VWarn warn = line_warnings.front();
        update(pos.row + rendered_rows, render_x + col, " ", 0, color, 0, 0, 1);
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
              update(pos.row + rendered_rows, render_x + col, err_sym, fg_color,
                     color, 0, 0, 1);
              col++;
              update(pos.row + rendered_rows, render_x + col, " ", fg_color,
                     color, 0, 0, 1);
              col++;
            }
          }
        }
        if (col < render_width) {
          update(pos.row + rendered_rows, render_x + col, " ", 0, 0 | color, 0,
                 0, 1);
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
          update(pos.row + rendered_rows, render_x + col, cluster.c_str(),
                 fg_color, color, 0, 0, width);
          col += width;
          warn_idx += cluster_len;
          while (width-- > 1)
            update(pos.row + rendered_rows, render_x + col - width, "\x1b",
                   fg_color, color, 0, 0, 0);
        }
      }
      while (col < render_width) {
        update(pos.row + rendered_rows, render_x + col, " ", 0, 0 | color, 0, 0,
               1);
        col++;
      }
      rendered_rows++;
    }
    global_byte_offset += line_len + 1;
    line_index++;
  }
  while (rendered_rows < this->size.row) {
    for (uint32_t col = 0; col < this->size.col; col++)
      update(pos.row + rendered_rows, pos.col + col, " ", 0xFFFFFF, 0, 0, 0, 1);
    rendered_rows++;
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
    // if (this->completion.active && !this->completion.box.hidden)
    //   this->completion.box.render(cursor);
    // else if (this->hover_active)
    //   this->hover.render(cursor);
    // else if (this->diagnostics_active)
    //   this->diagnostics.render(cursor);
    if (this->hover_active)
      ui::hover_popup->pos = cursor;
  }
  free(it->buffer);
  free(it);
  if (this->parser)
    this->parser->scroll(line_index + 5);
}

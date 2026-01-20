#include "ui/completionbox.h"
#include "editor/completions.h"
#include "io/sysio.h"
#include "utils/utils.h"

std::string item_kind_name(uint8_t kind) {
  switch (kind) {
  case 1:
    return "Text";
  case 2:
    return "Method";
  case 3:
    return "Function";
  case 4:
    return "Constructor";
  case 5:
    return "Field";
  case 6:
    return "Variable";
  case 7:
    return "Class";
  case 8:
    return "Interface";
  case 9:
    return "Module";
  case 10:
    return "Property";
  case 11:
    return "Unit";
  case 12:
    return "Value";
  case 13:
    return "Enum";
  case 14:
    return "Keyword";
  case 15:
    return "Snippet";
  case 16:
    return "Color";
  case 17:
    return "File";
  case 18:
    return "Reference";
  case 19:
    return "Folder";
  case 20:
    return "EnumMember";
  case 21:
    return "Constant";
  case 22:
    return "Struct";
  case 23:
    return "Event";
  case 24:
    return "Operator";
  case 25:
    return "TypeParameter";
  default:
    return "Unknown";
  }
}

const char *item_symbol(uint8_t kind) { return "●"; }

uint32_t kind_color(uint8_t kind) { return 0x82AAFF; }

void CompletionBox::render_update() {
  if (hidden || session->visible.empty())
    return;
  std::unique_lock lock(mtx);
  uint32_t max_label_len = 0;
  uint32_t max_detail_len = 0;
  uint32_t max_kind_len = 0;
  for (auto i : session->visible) {
    if (i >= session->items.size())
      continue;
    auto &item = session->items[i];
    max_label_len = MAX(max_label_len, (uint32_t)item.label.size());
    if (item.detail)
      max_detail_len = MAX(max_detail_len, (uint32_t)item.detail->size());
    max_kind_len =
        MAX(max_kind_len, (uint32_t)item_kind_name(item.kind).size());
  }
  size.row = session->visible.size() + 2;
  size.col = 2 + 2 + max_label_len + 1 + max_detail_len + 2 + max_kind_len + 1;
  cells.assign(size.row * size.col, {" ", 0, 0, 0, 0, 0});
  auto set = [&](uint32_t r, uint32_t c, const char *text, uint32_t fg,
                 uint32_t bg, uint8_t flags) {
    if (r < size.row && c < size.col)
      cells[r * size.col + c] = {std::string(text), 0, fg, bg, flags, 0};
  };
  uint32_t border_fg = 0x82AAFF;
  uint32_t sel_bg = 0x174225;
  set(0, 0, "┌", border_fg, 0, 0);
  for (uint32_t c = 1; c < size.col - 1; c++)
    set(0, c, "─", border_fg, 0, 0);
  set(0, size.col - 1, "┐", border_fg, 0, 0);
  for (uint32_t row_idx = 0; row_idx < session->visible.size(); row_idx++) {
    uint32_t r = row_idx + 1;
    auto &item = session->items[session->visible[row_idx]];
    uint32_t bg = (session->visible[row_idx] == session->select) ? sel_bg : 1;
    uint32_t fg = 0xFFFFFF;
    set(r, 0, "│", border_fg, 0, 0);
    uint32_t c = 1;
    const char *sym = item_symbol(item.kind);
    set(r, c++, sym, kind_color(item.kind), bg, 0);
    set(r, c++, " ", fg, bg, 0);
    for (size_t i = 0; i < item.label.size(); i++)
      set(r, c + i, (char[2]){item.label[i], 0}, fg, bg,
          item.deprecated ? CF_STRIKETHROUGH : 0);
    c += item.label.size();
    set(r, c++, " ", fg, bg, 0);
    uint32_t detail_fg = 0xAAAAAA;
    if (item.detail) {
      for (size_t i = 0; i < item.detail->size(); i++)
        set(r, c + i, (char[2]){(*item.detail)[i], 0}, detail_fg, bg, 0);
      c += item.detail->size();
    }
    uint32_t pad = size.col - 1 - c - max_kind_len;
    for (uint32_t i = 0; i < pad; i++)
      set(r, c + i, " ", fg, bg, 0);
    c += pad;
    std::string kind_name = item_kind_name(item.kind);
    for (size_t i = 0; i < kind_name.size(); i++)
      set(r, c + i, (char[2]){kind_name[i], 0}, kind_color(item.kind), bg, 0);
    set(r, size.col - 1, "│", border_fg, 0, 0);
  }
  uint32_t bottom = size.row - 1;
  set(bottom, 0, "└", border_fg, 0, 0);
  for (uint32_t c = 1; c < size.col - 1; c++)
    set(bottom, c, "─", border_fg, 0, 0);
  set(bottom, size.col - 1, "┘", border_fg, 0, 0);
}

void CompletionBox::render(Coord pos) {
  if (hidden || session->visible.empty())
    return;
  std::shared_lock lock(mtx);
  int32_t start_row = (int32_t)pos.row - (int32_t)size.row;
  if (start_row < 0)
    start_row = pos.row + 1;
  int32_t start_col = pos.col;
  Coord screen_size = get_size();
  if (start_col + size.col > screen_size.col) {
    start_col = screen_size.col - size.col;
    if (start_col < 0)
      start_col = 0;
  }
  position = {(uint32_t)start_row, (uint32_t)start_col};
  for (uint32_t r = 0; r < size.row; r++)
    for (uint32_t c = 0; c < size.col; c++)
      update(start_row + r, start_col + c, cells[r * size.col + c].utf8,
             cells[r * size.col + c].fg, cells[r * size.col + c].bg,
             cells[r * size.col + c].flags);
  if (session->items.size() > session->select &&
      session->items[session->select].documentation &&
      *session->items[session->select].documentation != "" &&
      !session->hover_dirty) {
    if (session->doc != session->select) {
      session->doc = session->select;
      session->hover.clear();
      session->hover.text = *session->items[session->select].documentation;
      session->hover.is_markup = true;
      session->hover_dirty = true;
    } else {
      if ((int32_t)position.col - (int32_t)session->hover.size.col > 0) {
        session->hover.render({position.row + session->hover.size.row,
                               position.col - session->hover.size.col});
      } else {
        session->hover.render(
            {position.row + session->hover.size.row, position.col + size.col});
      }
    }
  }
}

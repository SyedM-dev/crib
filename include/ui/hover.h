#ifndef UI_HOVER_H
#define UI_HOVER_H

#include "editor/decl.h"
#include "io/sysio.h"
#include "pch.h"
#include "ts/decl.h"
#include "utils/utils.h"

struct HoverBox {
  std::string text;
  std::atomic<bool> is_markup;
  uint32_t scroll_;
  std::vector<ScreenCell> cells;
  uint32_t box_width;
  uint32_t box_height;
  std::vector<Highlight> highlights;
  std::vector<Span> hover_spans;

  void clear();
  void scroll(int32_t number);
  void render_first(bool scroll = false);
  void render(Coord pos);
};

#endif

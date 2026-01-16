#ifndef UI_HOVER_H
#define UI_HOVER_H

#include "editor/decl.h"
#include "io/sysio.h"
#include "pch.h"
#include "utils/utils.h"

struct HoverBox {
  std::string text;
  std::atomic<bool> is_markup;
  uint32_t scroll_;
  std::vector<ScreenCell> cells;
  Coord size;

  void clear();
  void scroll(int32_t number);
  void render_first(bool scroll = false);
  void render(Coord pos);
};

#endif

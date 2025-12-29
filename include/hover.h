#ifndef HOVER_H
#define HOVER_H

#include "./pch.h"
#include "./spans.h"
#include "./ts_def.h"
#include "./ui.h"
#include "./utils.h"

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

struct DiagnosticBox {
  std::vector<VWarn> warnings;
  std::vector<ScreenCell> cells;
  uint32_t box_width;
  uint32_t box_height;

  void clear();
  void render_first();
  void render(Coord pos);
};

#endif

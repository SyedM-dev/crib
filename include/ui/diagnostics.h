#ifndef UI_DIAGNOSTICS_H
#define UI_DIAGNOSTICS_H

#include "editor/decl.h"
#include "io/sysio.h"
#include "pch.h"
#include "utils/utils.h"

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

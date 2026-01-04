#ifndef UI_DIAGNOSTICS_H
#define UI_DIAGNOSTICS_H

#include "editor/decl.h"
#include "io/sysio.h"
#include "pch.h"
#include "utils/utils.h"

struct DiagnosticBox {
  std::vector<VWarn> warnings;
  std::vector<ScreenCell> cells;
  Coord size;

  void clear();
  void render_first();
  void render(Coord pos);
};

#endif

#ifndef UI_BAR_H
#define UI_BAR_H

#include "editor/editor.h"
#include "io/sysio.h"
#include "utils/utils.h"

struct Bar {
  Coord screen;
  std::string command = "";
  uint32_t cursor = 0;

  Bar(Coord screen) : screen(screen) {}
  void render();
  void handle(KeyEvent event);
  void log(std::string message);
};

#endif

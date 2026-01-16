#ifndef EDITOR_DECL_H
#define EDITOR_DECL_H

#include "utils/utils.h"

struct TextEdit {
  Coord start;
  Coord end;
  std::string text;
};

struct VWarn {
  uint32_t line;
  std::string text;
  std::string text_full;
  std::string source;
  std::string code;
  std::vector<std::string> see_also;
  int8_t type;
  uint32_t start;
  uint32_t end{UINT32_MAX};

  bool operator<(const VWarn &other) const { return line < other.line; }
};

struct VAI {
  Coord pos;
  char *text;
  uint32_t len;
  uint32_t lines; // number of \n in text for speed .. the ai part will not
                  // line wrap but multiline ones need to have its own lines
                  // after the first one
};

#endif

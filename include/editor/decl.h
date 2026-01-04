#ifndef EDITOR_DECL_H
#define EDITOR_DECL_H

#include "utils/utils.h"

struct TextEdit {
  // NOTE: start.col is in utf16 index and not clusters or utf8
  Coord start;
  // NOTE: end.col is in utf16 index and not clusters or utf8
  Coord end;
  std::string text;
};

struct Fold {
  uint32_t start;
  uint32_t end;

  bool contains(uint32_t line) const { return line >= start && line <= end; }
  bool operator<(const Fold &other) const { return start < other.start; }
};

struct Span {
  uint32_t start;
  uint32_t end;
  Highlight *hl;

  bool operator<(const Span &other) const { return start < other.start; }
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

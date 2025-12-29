#ifndef SPANS_H
#define SPANS_H

#include "./pch.h"
#include "./utils.h"

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

struct Spans {
  std::vector<Span> spans;
  Queue<std::pair<uint32_t, int64_t>> edits;
  bool mid_parse = false;
  std::shared_mutex mtx;
};

struct SpanCursor {
  Spans &spans;
  size_t index = 0;
  std::vector<Span *> active;
  std::shared_lock<std::shared_mutex> lock;

  SpanCursor(Spans &s) : spans(s) {}
  Highlight *get_highlight(uint32_t byte_offset) {
    for (int i = (int)active.size() - 1; i >= 0; i--)
      if (active[i]->end <= byte_offset)
        active.erase(active.begin() + i);
    while (index < spans.spans.size() &&
           spans.spans[index].start <= byte_offset) {
      if (spans.spans[index].end > byte_offset)
        active.push_back(const_cast<Span *>(&spans.spans[index]));
      index++;
    }
    Highlight *best = nullptr;
    int max_prio = -1;
    for (auto *s : active)
      if (s->hl->priority > max_prio) {
        max_prio = s->hl->priority;
        best = s->hl;
      }
    return best;
  }
  void sync(uint32_t byte_offset) {
    lock = std::shared_lock(spans.mtx);
    active.clear();
    size_t left = 0, right = spans.spans.size();
    while (left < right) {
      size_t mid = (left + right) / 2;
      if (spans.spans[mid].start <= byte_offset)
        left = mid + 1;
      else
        right = mid;
    }
    index = left;
    while (left > 0) {
      left--;
      if (spans.spans[left].end > byte_offset)
        active.push_back(const_cast<Span *>(&spans.spans[left]));
      else if (byte_offset - spans.spans[left].end > 1000)
        break;
    }
  }
};

#endif

#ifndef EDITOR_SPANS_H
#define EDITOR_SPANS_H

#include "editor/decl.h"
#include "utils/utils.h"

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

inline void apply_edit(std::vector<Span> &spans, uint32_t x, int64_t y) {
  Span key{.start = x, .end = 0, .hl = nullptr};
  auto it = std::lower_bound(
      spans.begin(), spans.end(), key,
      [](const Span &a, const Span &b) { return a.start < b.start; });
  size_t idx = std::distance(spans.begin(), it);
  while (idx > 0 && spans.at(idx - 1).end >= x)
    --idx;
  for (size_t i = idx; i < spans.size();) {
    Span &s = spans.at(i);
    if (s.start < x && s.end >= x) {
      s.end += y;
    } else if (s.start > x) {
      s.start += y;
      s.end += y;
    }
    if (s.end <= s.start)
      spans.erase(spans.begin() + i);
    else
      ++i;
  }
}

#endif

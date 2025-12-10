#ifndef EDITOR_H
#define EDITOR_H

#include "../libs/tree-sitter/lib/include/tree_sitter/api.h"
#include "./rope.h"
#include "./ui.h"
#include "./utils.h"
#include <algorithm>
#include <cstdint>
#include <map>
#include <shared_mutex>
#include <unordered_map>

struct Highlight {
  uint32_t fg;
  uint32_t bg;
  uint32_t flags;
  uint8_t priority;
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

struct Editor {
  const char *filename;                 // Filename of the editor
  Knot *root;                           // A rope
  std::shared_mutex knot_mtx;           // A mutex
  Coord cursor;                         // position of the cursor
  uint32_t cursor_preffered;            // preffered visual column
  Coord selection;                      // position of the selection
  bool selection_active;                // true if there is a selection
  Coord position;                       // Position of the editor
  Coord size;                           // Size of the editor
  Coord scroll;                         // Position of the scroll
  TSTree *tree;                         // Tree-sitter tree
  TSParser *parser;                     // Tree-sitter parser
  TSQuery *query;                       // Tree-sitter query
  const TSLanguage *language;           // Tree-sitter language
  Queue<TSInputEdit> edit_queue;        // Tree-sitter edit queue
  std::vector<Highlight> query_map;     // Tree-sitter query map
  std::vector<int8_t> folded;           // folded lines indexed by line number
  Spans spans;                          // Highlighted spans
  std::map<uint32_t, bool> folded_node; // maps content hash to fold state
                                        // - built by tree-sitter helpers
};

Editor *new_editor(const char *filename, Coord position, Coord size);
void free_editor(Editor *editor);
void render_editor(Editor *editor);
void fold(Editor *editor, uint32_t start_line, uint32_t end_line);
void scroll_up(Editor *editor, uint32_t lines);
void scroll_down(Editor *editor, uint32_t lines);
void cursor_up(Editor *editor, uint32_t number);
void cursor_down(Editor *editor, uint32_t number);
void cursor_left(Editor *editor, uint32_t number);
void cursor_right(Editor *editor, uint32_t number);
void ensure_scroll(Editor *editor);
void apply_edit(std::vector<Span> &spans, uint32_t x, int64_t y);

#endif

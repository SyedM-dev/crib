#ifndef EDITOR_H
#define EDITOR_H

#include "../libs/tree-sitter/lib/include/tree_sitter/api.h"
#include "./knot.h"
#include "./ui.h"
#include "./utils.h"
#include <algorithm>
#include <cstdint>
#include <map>
#include <shared_mutex>
#include <unordered_map>

#define CHAR 0
#define WORD 1
#define LINE 2

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
  const char *filename;
  Knot *root;
  std::shared_mutex knot_mtx;
  Coord cursor;
  uint32_t cursor_preffered;
  Coord selection;
  bool selection_active;
  int selection_type;
  Coord position;
  Coord size;
  Coord scroll;
  TSTree *tree;
  TSParser *parser;
  TSQuery *query;
  const TSLanguage *language;
  Queue<TSInputEdit> edit_queue;
  std::vector<Highlight> query_map;
  std::vector<int8_t> folded;
  Spans spans;
  Spans def_spans;
  std::map<uint32_t, bool> folded_node;
};

Editor *new_editor(const char *filename, Coord position, Coord size);
void free_editor(Editor *editor);
void render_editor(Editor *editor);
void fold(Editor *editor, uint32_t start_line, uint32_t end_line);
void cursor_up(Editor *editor, uint32_t number);
void cursor_down(Editor *editor, uint32_t number);
Coord move_left(Editor *editor, Coord cursor, uint32_t number);
Coord move_right(Editor *editor, Coord cursor, uint32_t number);
void cursor_left(Editor *editor, uint32_t number);
void cursor_right(Editor *editor, uint32_t number);
void scroll_up(Editor *editor, uint32_t number);
void scroll_down(Editor *editor, uint32_t number);
void ensure_cursor(Editor *editor);
void ensure_scroll(Editor *editor);
void handle_editor_event(Editor *editor, KeyEvent event);
void apply_edit(std::vector<Span> &spans, uint32_t x, int64_t y);
void edit_erase(Editor *editor, Coord pos, int64_t len);
void edit_insert(Editor *editor, Coord pos, char *data, uint32_t len);
Coord editor_hit_test(Editor *editor, uint32_t x, uint32_t y);
char *get_selection(Editor *editor, uint32_t *out_len);
void editor_worker(Editor *editor);
void word_boundaries(Editor *editor, Coord coord, uint32_t *prev_col,
                     uint32_t *next_col, uint32_t *prev_clusters,
                     uint32_t *next_clusters);
void word_boundaries_exclusive(Editor *editor, Coord coord, uint32_t *prev_col,
                               uint32_t *next_col);

#endif

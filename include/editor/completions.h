#ifndef EDITOR_COMPLETIONS_H
#define EDITOR_COMPLETIONS_H

#include "pch.h"
#include "ui/completionbox.h"
#include "ui/hover.h"
#include "utils/utils.h"

struct CompletionItem {
  std::string label;
  uint8_t kind;
  std::optional<std::string> detail;
  std::optional<std::string> documentation;
  bool is_markup = false;
  bool deprecated = false;
  bool asis = true;
  std::string sort;
  std::string filter;
  bool snippet = false;
  std::vector<TextEdit> edits;
  json original;
  std::vector<char> end_chars;
};

struct CompletionSession {
  std::shared_mutex mtx;
  bool active = false;
  Coord hook;
  std::optional<std::string> prefix;
  uint8_t select = 0;
  std::vector<CompletionItem> items;
  std::vector<uint8_t> visible;
  bool complete = true;
  std::optional<char> trigger_char;
  uint8_t trigger = 0;
  CompletionBox box;
  HoverBox hover;
  uint32_t doc = UINT32_MAX;
  std::atomic<bool> hover_dirty = false;
  int version;

  CompletionSession() : box(this) {}
};

#endif

#ifndef EDITOR_COMPLETIONS_H
#define EDITOR_COMPLETIONS_H

#include "editor/decl.h"
#include "pch.h"
#include "ui/completionbox.h"
#include "utils/utils.h"

struct CompletionItem {
  std::string label;                        // Shown in the autocomplete box
  uint8_t kind;                             // Function, variable, class, etc.
  std::optional<std::string> detail;        // Shown greyed in autocomplete box
  std::optional<std::string> documentation; // Hover box (can be lazy-loaded)
  bool is_markup = false;
  bool deprecated = false; // Shown with strikethrough, may push down in list
  std::string sort;        // Used for sorting
  std::string filter;      // Used for filtering (default: label)
  bool snippet = false;
  std::vector<TextEdit> edits;
  json original;
  std::vector<char> end_chars; // Ends completion session if typed
};

struct CompletionSession {
  std::shared_mutex mtx;

  bool active = false;
  Coord hook;                        // set to start of word
  std::optional<std::string> prefix; // text between hook and cursor
  uint8_t select = 0; // index of selected item (defualts to preselcted one
                      // when data requested)
  std::vector<CompletionItem> items;
  std::vector<uint8_t> visible;
  bool complete = true; // If false, client may request more items on filter
                        // (but doesnt try filtering on its own)
  std::optional<char> trigger_char; // Character that triggered completion sent
                                    // to lsp for isIncomplete resolving
  uint8_t trigger = 0; // Type of trigger (1: manual, 2: trigger char, 3: auto)
  CompletionBox box;

  CompletionSession() : box(this) {}
};

#endif

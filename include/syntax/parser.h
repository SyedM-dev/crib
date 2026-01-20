#ifndef SYNTAX_PARSER_H
#define SYNTAX_PARSER_H

#include "syntax/decl.h"
#include "syntax/line_tree.h"

struct Parser {
  struct Editor *editor = nullptr;
  std::string lang;
  std::shared_ptr<void> (*parse_func)(std::vector<Token> *tokens,
                                      std::shared_ptr<void> in_state,
                                      const char *text, uint32_t len);
  bool (*state_match_func)(std::shared_ptr<void> state_1,
                           std::shared_ptr<void> state_2);
  std::atomic<uint32_t> scroll_max{UINT32_MAX - 2048};
  std::mutex mutex;
  std::mutex data_mutex;
  LineTree line_tree;
  std::set<uint32_t> dirty_lines;

  Parser(Editor *editor, std::string n_lang, uint32_t n_scroll_max);
  void edit(uint32_t start_line, uint32_t old_end_line, uint32_t inserted_rows);
  void work();
  void scroll(uint32_t line);
};

#endif

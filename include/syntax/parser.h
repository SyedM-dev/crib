#include "syntax/decl.h"

struct Parser {
  Knot *root;
  std::shared_mutex *knot_mutex;
  std::string lang;
  std::shared_ptr<void> (*parse_func)(std::vector<Token> *tokens,
                                      std::shared_ptr<void> in_state,
                                      const char *text, uint32_t len);
  bool (*state_match_func)(std::shared_ptr<void> state_1,
                           std::shared_ptr<void> state_2);
  std::atomic<uint32_t> scroll_max{UINT32_MAX - 2048};
  std::mutex mutex;
  std::mutex data_mutex;
  std::vector<LineData> line_data;
  std::set<uint32_t> dirty_lines;

  Parser(Knot *n_root, std::shared_mutex *n_knot_mutex, std::string n_lang,
         uint32_t n_scroll_max);
  void edit(Knot *n_root, uint32_t start_line, uint32_t old_end_line,
            uint32_t new_end_line);
  void work();
  void scroll(uint32_t line);
  uint8_t get_type(Coord c) {
    if (c.row >= line_data.size())
      return 0;
    const LineData &line = line_data[c.row];
    for (const Token &t : line.tokens)
      if (t.start <= c.col && c.col < t.end)
        return t.type;
    return 0;
  }
};

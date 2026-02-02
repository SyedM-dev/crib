#ifndef SCRIPTING_DECL_H
#define SCRIPTING_DECL_H

#include "syntax/decl.h"
#include "utils/utils.h"

namespace fs = std::filesystem;

extern std::unordered_map<std::string, std::pair<mrb_value, mrb_value>>
    custom_highlighters;
extern mrb_state *mrb;
extern fs::path ruby_config_path;

struct BarLight {
  uint32_t start;
  uint32_t end;
  Highlight highlight;
};

struct BarLine {
  std::string line;
  std::vector<BarLight> highlights;

  Highlight get_highlight(uint32_t x) {
    for (auto &hl : highlights) {
      if (hl.start <= x && x <= hl.end)
        return hl.highlight;
    }
    return {0xFFFFFF, 0, 0};
  }
};

void setup_ruby_bindings(mrb_state *mrb, RClass *C_module);
void ruby_start();
void ruby_shutdown();
void ruby_copy(const char *text, size_t len);
std::string ruby_paste();
std::string ruby_file_detect(std::string filename);
void load_theme();
void load_languages_info();
uint8_t read_line_endings();
void load_custom_highlighters();
bool custom_compare(mrb_value match_block, std::string state1,
                    std::string state2);
std::string parse_custom(std::vector<Token> *tokens, mrb_value parser_block,
                         const char *line, uint32_t len, std::string state,
                         uint32_t c_line);
BarLine bar_contents(uint8_t mode, std::string lang_name, uint32_t warnings,
                     std::string lsp_name, std::string filename,
                     std::string foldername, uint32_t line, uint32_t max_line,
                     uint32_t width);

#endif

#ifndef SCRIPTING_DECL_H
#define SCRIPTING_DECL_H

#include "syntax/decl.h"
#include "utils/utils.h"

extern std::unordered_map<std::string, std::pair<mrb_value, mrb_value>>
    custom_highlighters;

void ruby_start();
void ruby_shutdown();
void ruby_log(std::string msg);
void load_theme();
void load_languages_info();
uint8_t read_line_endings();
void load_custom_highlighters();
mrb_value parse_custom(std::vector<Token> *tokens, mrb_value parser_block,
                       const char *line, uint32_t len, mrb_value state,
                       uint32_t c_line);
bool custom_compare(mrb_value match_block, mrb_value state1, mrb_value state2);

#endif

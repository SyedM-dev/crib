#ifndef SCRIPTING_DECL_H
#define SCRIPTING_DECL_H

#include "syntax/decl.h"
#include "utils/utils.h"

extern std::unordered_map<std::string, std::pair<VALUE, VALUE>>
    custom_highlighters;

void ruby_start();
void ruby_shutdown();
void ruby_log(std::string msg);
void load_theme();
void load_languages_info();
uint8_t read_line_endings();
void load_custom_highlighters();
VALUE parse_custom(std::vector<Token> *tokens, VALUE parser_block,
                   const char *line, uint32_t len, VALUE state,
                   uint32_t line_num);
bool custom_compare(VALUE match_block, VALUE state1, VALUE state2);

#endif

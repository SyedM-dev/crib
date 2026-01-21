#ifndef SCRIPTING_DECL_H
#define SCRIPTING_DECL_H

#include "utils/utils.h"

void ruby_start(const char *main_file);
void ruby_shutdown();
void load_theme();
std::string read_line_endings();
std::string read_utf_mode();

#endif

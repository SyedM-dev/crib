#ifndef TS_H
#define TS_H

#include "./editor.h"
#include "./utils.h"
#include <pcre2.h>

#define HEX(s) (static_cast<uint32_t>(std::stoul(s, nullptr, 16)))

extern std::unordered_map<std::string, pcre2_code *> regex_cache;

TSQuery *load_query(const char *query_path, Editor *editor);
void ts_collect_spans(Editor *editor);
void clear_regex_cache();

#endif

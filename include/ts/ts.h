#ifndef TS_H
#define TS_H

#include "editor/editor.h"
#include "pch.h"
#include "utils/utils.h"

#define HEX(s) (static_cast<uint32_t>(std::stoul(s, nullptr, 16)))

extern std::unordered_map<std::string, pcre2_code *> regex_cache;

TSQuery *load_query(const char *query_path, TSSetBase *set);
void ts_collect_spans(Editor *editor);
bool ts_predicate(TSQuery *query, const TSQueryMatch &match,
                  std::function<std::string(const TSNode *)> subject_fn);
void clear_regex_cache();

#endif

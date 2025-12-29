#ifndef TS_H
#define TS_H

#include "./editor.h"
#include "./pch.h"
#include "./utils.h"

#define HEX(s) (static_cast<uint32_t>(std::stoul(s, nullptr, 16)))

extern std::unordered_map<std::string, pcre2_code *> regex_cache;

TSQuery *load_query(const char *query_path, TSSetBase *set);
void ts_collect_spans(Editor *editor);
bool ts_predicate(TSQuery *query, const TSQueryMatch &match,
                  std::function<std::string(const TSNode *)> subject_fn);
void clear_regex_cache();
template <typename T>
inline T *safe_get(std::map<uint16_t, T> &m, uint16_t key) {
  auto it = m.find(key);
  if (it == m.end())
    return nullptr;
  return &it->second;
}

#endif

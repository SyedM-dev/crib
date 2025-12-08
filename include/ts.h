#ifndef TS_H
#define TS_H

#include "./editor.h"

#define HEX(s) (static_cast<uint32_t>(std::stoul(s, nullptr, 16)))

TSQuery *load_query(const char *query_path, Editor *editor);
void ts_collect_spans(Editor *editor);

#endif

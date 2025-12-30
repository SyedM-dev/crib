#include "config.h"
#include "io/sysio.h"
#include "ts/ts.h"

std::unordered_map<std::string, pcre2_code *> regex_cache;

static inline const TSNode *find_capture_node(const TSQueryMatch &match,
                                              uint32_t capture_id) {
  for (uint32_t i = 0; i < match.capture_count; i++)
    if (match.captures[i].index == capture_id)
      return &match.captures[i].node;
  return nullptr;
}

void clear_regex_cache() {
  for (auto &kv : regex_cache)
    pcre2_code_free(kv.second);
  regex_cache.clear();
}

pcre2_code *get_re(const std::string &pattern) {
  auto it = regex_cache.find(pattern);
  if (it != regex_cache.end())
    return it->second;
  int errornum;
  PCRE2_SIZE erroffset;
  pcre2_code *re =
      pcre2_compile((PCRE2_SPTR)pattern.c_str(), PCRE2_ZERO_TERMINATED, 0,
                    &errornum, &erroffset, nullptr);
  regex_cache[pattern] = re;
  return re;
}

TSQuery *load_query(const char *query_path, TSSetBase *set) {
  const TSLanguage *lang = set->language;
  std::ifstream file(query_path, std::ios::in | std::ios::binary);
  if (!file.is_open())
    return nullptr;
  std::string highlight_query((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  int errornumber = 0;
  PCRE2_SIZE erroroffset = 0;
  pcre2_code *re = pcre2_compile(
      (PCRE2_SPTR) R"((@[A-Za-z0-9_.]+)|(;; \#[0-9a-fA-F]{6} \#[0-9a-fA-F]{6} [01] [01] [01] [01] \d+)|(;; !(\w+)))",
      PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, nullptr);
  if (!re)
    return nullptr;
  pcre2_match_data *match_data =
      pcre2_match_data_create_from_pattern(re, nullptr);
  std::map<std::string, int> capture_name_cache;
  Highlight *c_hl = nullptr;
  Language c_lang = {"unknown", nullptr, 0};
  int i = 0;
  PCRE2_SIZE offset = 0;
  PCRE2_SIZE subject_length = highlight_query.size();
  while (offset < subject_length) {
    int rc = pcre2_match(re, (PCRE2_SPTR)highlight_query.c_str(),
                         subject_length, offset, 0, match_data, nullptr);
    if (rc <= 0)
      break;
    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
    std::string mct =
        highlight_query.substr(ovector[0], ovector[1] - ovector[0]);
    if (!mct.empty() && mct[0] == '@') {
      std::string capture_name = mct;
      if (!capture_name_cache.count(capture_name)) {
        if (c_hl) {
          set->query_map[i] = *c_hl;
          delete c_hl;
          c_hl = nullptr;
        }
        if (c_lang.fn != nullptr) {
          set->injection_map[i] = c_lang;
          c_lang = {"unknown", nullptr, 0};
        }
        capture_name_cache[capture_name] = i;
        i++;
      }
    } else if (mct.substr(0, 4) == ";; #") {
      if (c_hl)
        delete c_hl;
      c_hl = new Highlight();
      c_hl->fg = HEX(mct.substr(4, 6));
      c_hl->bg = HEX(mct.substr(12, 6));
      int bold = std::stoi(mct.substr(19, 1));
      int italic = std::stoi(mct.substr(21, 1));
      int underline = std::stoi(mct.substr(23, 1));
      int strike = std::stoi(mct.substr(25, 1));
      c_hl->priority = std::stoi(mct.substr(27));
      c_hl->flags = (bold ? CF_BOLD : 0) | (italic ? CF_ITALIC : 0) |
                    (underline ? CF_UNDERLINE : 0) |
                    (strike ? CF_STRIKETHROUGH : 0);
    } else if (mct.substr(0, 4) == ";; !") {
      auto it = kLanguages.find(mct.substr(4));
      if (it != kLanguages.end())
        c_lang = it->second;
      else
        c_lang = {"unknown", nullptr, 0};
    }
    offset = ovector[1];
  }
  if (c_hl)
    delete c_hl;
  pcre2_match_data_free(match_data);
  pcre2_code_free(re);
  uint32_t error_offset = 0;
  TSQueryError error_type = (TSQueryError)0;
  TSQuery *q = ts_query_new(lang, highlight_query.c_str(),
                            (uint32_t)highlight_query.length(), &error_offset,
                            &error_type);
  if (!q)
    log("Failed to create TSQuery at offset %u, error type %d", error_offset,
        (int)error_type);
  return q;
}

bool ts_predicate(TSQuery *query, const TSQueryMatch &match,
                  std::function<std::string(const TSNode *)> subject_fn) {
  uint32_t step_count;
  const TSQueryPredicateStep *steps =
      ts_query_predicates_for_pattern(query, match.pattern_index, &step_count);
  if (!steps || step_count != 4)
    return true;
  std::string command;
  std::string regex_txt;
  uint32_t subject_id = 0;
  for (uint32_t i = 0; i < step_count; i++) {
    const TSQueryPredicateStep *step = &steps[i];
    if (step->type == TSQueryPredicateStepTypeDone)
      break;
    switch (step->type) {
    case TSQueryPredicateStepTypeString: {
      uint32_t length = 0;
      const char *s =
          ts_query_string_value_for_id(query, step->value_id, &length);
      if (i == 0)
        command.assign(s, length);
      else
        regex_txt.assign(s, length);
      break;
    }
    case TSQueryPredicateStepTypeCapture: {
      subject_id = step->value_id;
      break;
    }
    case TSQueryPredicateStepTypeDone:
      break;
    }
  }
  const TSNode *node = find_capture_node(match, subject_id);
  pcre2_code *re = get_re(regex_txt);
  std::string subject = subject_fn(node);
  pcre2_match_data *md = pcre2_match_data_create_from_pattern(re, nullptr);
  int rc = pcre2_match(re, (PCRE2_SPTR)subject.c_str(), subject.size(), 0, 0,
                       md, nullptr);
  pcre2_match_data_free(md);
  bool ok = (rc >= 0);
  return (command == "match?" ? ok : !ok);
}

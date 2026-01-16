#ifndef SYNTAX_LANGS_H
#define SYNTAX_LANGS_H

#include "syntax/decl.h"

#define DEF_LANG(name)                                                         \
  std::shared_ptr<void> name##_parse(std::vector<Token> *tokens,               \
                                     std::shared_ptr<void> in_state,           \
                                     const char *text, uint32_t len);          \
  bool name##_state_match(std::shared_ptr<void> state_1,                       \
                          std::shared_ptr<void> state_2);

#define LANG_A(name) {name##_parse, name##_state_match}

DEF_LANG(ruby);

inline static const std::unordered_map<
    std::string,
    std::tuple<std::shared_ptr<void> (*)(std::vector<Token> *tokens,
                                         std::shared_ptr<void> in_state,
                                         const char *text, uint32_t len),
               bool (*)(std::shared_ptr<void> state_1,
                        std::shared_ptr<void> state_2)>>
    parsers = {
        {"ruby", LANG_A(ruby)},
};

#endif

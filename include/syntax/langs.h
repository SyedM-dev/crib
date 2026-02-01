#ifndef SYNTAX_LANGS_H
#define SYNTAX_LANGS_H

#include "scripting/decl.h"
#include "syntax/decl.h"

#define DEF_LANG(name)                                                         \
  std::shared_ptr<void> name##_parse(                                          \
      std::vector<Token> *tokens, std::shared_ptr<void> in_state,              \
      const char *text, uint32_t len, uint32_t line_num);                      \
  bool name##_state_match(std::shared_ptr<void> state_1,                       \
                          std::shared_ptr<void> state_2);

#define LANG_A(name)                                                           \
  {                                                                            \
    #name, { name##_parse, name##_state_match }                                \
  }

template <typename T>
inline std::shared_ptr<T> ensure_state(std::shared_ptr<T> state) {
  using U = typename T::full_state_type;
  if (!state)
    state = std::make_shared<T>();
  if (!state.unique())
    state = std::make_shared<T>(*state);
  if (!state->full_state)
    state->full_state = std::make_shared<U>();
  else if (!state->full_state.unique())
    state->full_state = std::make_shared<U>(*state->full_state);
  return state;
}

DEF_LANG(ruby);
DEF_LANG(bash);

inline static const std::unordered_map<
    std::string,
    std::tuple<std::shared_ptr<void> (*)(
                   std::vector<Token> *tokens, std::shared_ptr<void> in_state,
                   const char *text, uint32_t len, uint32_t line_num),
               bool (*)(std::shared_ptr<void> state_1,
                        std::shared_ptr<void> state_2)>>
    parsers = {
        LANG_A(ruby),
        LANG_A(bash),
};

#endif

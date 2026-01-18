#include "utils/utils.h"

bool compare(const char *a, const char *b, size_t n) {
  size_t i = 0;
  for (; i < n; ++i)
    if (a[i] != b[i])
      return false;
  return true;
}

std::string percent_decode(const std::string &s) {
  std::string out;
  out.reserve(s.size());
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '%' && i + 2 < s.size() && std::isxdigit(s[i + 1]) &&
        std::isxdigit(s[i + 2])) {
      auto hex = [](char c) -> int {
        if ('0' <= c && c <= '9')
          return c - '0';
        if ('a' <= c && c <= 'f')
          return c - 'a' + 10;
        if ('A' <= c && c <= 'F')
          return c - 'A' + 10;
        return 0;
      };
      char decoded = (hex(s[i + 1]) << 4) | hex(s[i + 2]);
      out.push_back(decoded);
      i += 2;
    } else {
      out.push_back(s[i]);
    }
  }
  return out;
}

std::string percent_encode(const std::string &s) {
  static const char *hex = "0123456789ABCDEF";
  std::string out;
  out.reserve(s.size() * 3);
  for (unsigned char c : s) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' ||
        c == '/') {
      out.push_back(c);
    } else {
      out.push_back('%');
      out.push_back(hex[c >> 4]);
      out.push_back(hex[c & 0xF]);
    }
  }
  return out;
}

std::string substitute_fence(const std::string &documentation,
                             const std::string &lang) {
  int errorcode;
  PCRE2_SIZE erroroffset;
  const char *pattern = "\n```\n(.*?)```\n";
  pcre2_code *re =
      pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, PCRE2_DOTALL,
                    &errorcode, &erroroffset, nullptr);
  if (!re)
    return documentation;
  PCRE2_SIZE outlen = 0;
  std::string replacement = "```" + lang + "\n$1```";
  int rc = pcre2_substitute(
      re, (PCRE2_SPTR)documentation.c_str(), documentation.size(), 0,
      PCRE2_SUBSTITUTE_GLOBAL | PCRE2_SUBSTITUTE_OVERFLOW_LENGTH, nullptr,
      nullptr, (PCRE2_SPTR)replacement.c_str(), replacement.size(), nullptr,
      &outlen);
  if (rc < 0) {
    pcre2_code_free(re);
    return documentation;
  }
  std::string out(outlen, '\0');
  rc = pcre2_substitute(re, (PCRE2_SPTR)documentation.c_str(),
                        documentation.size(), 0, PCRE2_SUBSTITUTE_GLOBAL,
                        nullptr, nullptr, (PCRE2_SPTR)replacement.c_str(),
                        replacement.size(), (PCRE2_UCHAR *)out.data(), &outlen);
  pcre2_code_free(re);
  if (rc < 0)
    return documentation;
  out.resize(outlen);
  return out;
}

std::string trim(const std::string &s) {
  size_t start = s.find_first_not_of(" \t\n\r");
  if (start == std::string::npos)
    return "";
  size_t end = s.find_last_not_of(" \t\n\r");
  return s.substr(start, end - start + 1);
}

std::string clean_text(const std::string &input) {
  std::string result = input;
  static const std::unordered_map<std::string, std::string> entities = {
      {"&nbsp;", " "}, {"&lt;", "<"},    {"&gt;", ">"},
      {"&amp;", "&"},  {"&quot;", "\""}, {"&apos;", "'"}};
  for (const auto &e : entities) {
    size_t pos = 0;
    while ((pos = result.find(e.first, pos)) != std::string::npos) {
      result.replace(pos, e.first.length(), e.second);
      pos += e.second.length();
    }
  }
  int errorcode;
  PCRE2_SIZE erroroffset;
  pcre2_code *re =
      pcre2_compile((PCRE2_SPTR) "(\n\\s*)+", PCRE2_ZERO_TERMINATED, 0,
                    &errorcode, &erroroffset, nullptr);
  if (!re)
    return result;
  pcre2_match_data *match_data =
      pcre2_match_data_create_from_pattern(re, nullptr);
  PCRE2_SIZE offset = 0;
  std::string clean;
  while (offset < result.size()) {
    int rc = pcre2_match(re, (PCRE2_SPTR)result.c_str(), result.size(), offset,
                         0, match_data, nullptr);
    if (rc < 0) {
      clean += result.substr(offset);
      break;
    }
    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
    clean += result.substr(offset, ovector[0] - offset) + "\n";
    offset = ovector[1];
  }
  pcre2_match_data_free(match_data);
  pcre2_code_free(re);
  std::string final_str;
  size_t start = 0;
  while (start < clean.size()) {
    size_t end = clean.find('\n', start);
    if (end == std::string::npos)
      end = clean.size();
    std::string line = clean.substr(start, end - start);
    size_t first = line.find_first_not_of(" \t\r");
    size_t last = line.find_last_not_of(" \t\r");
    if (first != std::string::npos)
      final_str += line.substr(first, last - first + 1) + "\n";
    start = end + 1;
  }
  if (!final_str.empty() && final_str.back() == '\n')
    final_str.pop_back();
  return final_str;
}

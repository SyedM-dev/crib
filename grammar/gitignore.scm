;; #99ADBF #000000 0 1 0 0 1
(comment) @comment @spell

;; #7dcfff #000000 0 0 0 0 2
(pattern_char) @string.special.path

;; #AAD94C #000000 0 0 0 0 3
[
  (directory_separator)
  (directory_separator_escaped)
] @punctuation.delimiter

;; #F29668 #000000 0 0 0 0 1
[
  (wildcard_char_single)
  (wildcard_chars)
  (wildcard_chars_allow_slash)
] @character.special

;; #FF8F40 #000000 0 0 0 0 1
[
  (pattern_char_escaped)
  (bracket_char_escaped)
] @string.escape

;; #AAD94C #000000 0 0 0 0 3
(negation) @punctuation.special

;; #F07178 #000000 0 0 0 0 1
(bracket_negation) @operator

;; #AAD94C #000000 0 0 0 0 3
[
  "["
  "]"
] @punctuation.bracket

;; #7dcfff #000000 0 0 0 0 2
(bracket_char) @constant

;; #F29668 #000000 0 0 0 0 1
(bracket_range
  "-" @operator)

;; #F07178 #000000 0 0 0 0 1
(bracket_char_class) @constant.builtin

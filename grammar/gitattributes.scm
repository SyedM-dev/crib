;; #AAD94C #000000 0 0 0 3
(dir_sep) @punctuation.delimiter

;; #AAD94C #000000 0 0 0 3
(quoted_pattern
  "\"" @punctuation.special)

;; #AAD94C #000000 0 0 0 3
(range_notation) @string.special

;; #AAD94C #000000 0 0 0 3
(range_notation
  [ "[" "]" ] @punctuation.bracket)

;; #F29668 #000000 0 0 0 1
(wildcard) @string.regexp

;; #F29668 #000000 0 0 0 1
(range_negation) @operator

;; #7dcfff #000000 0 0 0 2
(character_class) @constant

;; #F29668 #000000 0 0 0 1
(class_range "-" @operator)

;; #FF8F40 #000000 0 0 0 1
[
  (ansi_c_escape)
  (escaped_char)
] @escape

;; #AAD94C #000000 0 0 0 3
(attribute
  (attr_name) @variable.parameter)

;; #F07178 #000000 0 0 0 1
(attribute
  (builtin_attr) @variable.builtin)

;; #F29668 #000000 0 0 0 1
[
  (attr_reset)
  (attr_unset)
  (attr_set)
] @operator

;; #F29668 #000000 0 0 0 1
(boolean_value) @boolean

;; #AAD94C #000000 0 0 0 2
(string_value) @string

;; #D2A6FF #000000 0 0 0 1
(macro_tag) @keyword

;; #7dcfff #000000 0 0 0 2
(macro_def
  macro_name: (_) @property)

;; #F07178 #000000 0 0 0 1
[
  (pattern_negation)
  (redundant_escape)
  (trailing_slash)
  (ignored_value)
] @error

;; #99ADBF #000000 0 1 0 1
(comment) @comment

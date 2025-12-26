;; ============================================================
;; Punctuation / brackets
;; ============================================================

;; #B6BEC8 #000000 0 0 0 1
[
  "("
  ")"
  "(?"
  "(?:"
  "(?<"
  "(?P<"
  "(?P="
  ">"
  "["
  "]"
  "{"
  "}"
  "[:"
  ":]"
] @punctuation.bracket

;; #F29CC3 #000000 0 0 0 2
(group_name) @property

;; #F29668 #000000 0 0 0 1
[
  "*"
  "+"
  "?"
  "|"
  "="
  "!"
] @operator

;; #B8E986 #000000 0 0 0 2
(count_quantifier
  [
    (decimal_digits) @number
    "," @punctuation.delimiter
  ])

;; #F29668 #000000 0 0 0 2
(inline_flags_group
  "-"? @operator
  ":"? @punctuation.delimiter)

;; #F29CC3 #000000 0 0 0 2
(flags) @character.special

;; #C2E8FF #000000 0 0 0 2
(character_class
  [
    "^" @operator
    (class_range "-" @operator)
  ])

;; #D2A6FF #000000 0 0 0 2
[
  (class_character)
  (posix_class_name)
] @constant.character

;; #D2A6FF #000000 0 0 0 2
(pattern_character) @string

;; #95E6CB #000000 0 0 0 2
[
  (identity_escape)
  (control_letter_escape)
  (character_class_escape)
  (control_escape)
  (start_assertion)
  (end_assertion)
  (boundary_assertion)
  (non_boundary_assertion)
] @escape

;; ============================================================
;; Strings & escapes
;; ============================================================

;; #AAD94C #000000 0 0 0 0 2
(string) @string

;; #95E6CB #000000 0 0 0 0 2
(escape_sequence) @string.escape

;; ============================================================
;; Identifiers
;; ============================================================

;; #C4B5FF #000000 0 0 0 0 2
(capture
  (identifier) @type)

;; #FFB454 #000000 0 0 0 0 2
(predicate
  name: (identifier) @function.call)

;; #F29CC3 #000000 0 0 0 0 2
(named_node
  name: (identifier) @variable)

;; #F29CC3 #000000 0 0 0 0 2
(missing_node
  name: (identifier) @variable)

;; #F07178 #000000 0 0 0 0 2
(field_definition
  name: (identifier) @variable.member)

;; #F29CC3 #000000 0 0 0 0 2
(negated_field
  "!" @operator
  (identifier) @property)

;; ============================================================
;; Comments
;; ============================================================

;; #99ADBF #000000 0 1 0 0 2
(comment) @comment @spell

;; ============================================================
;; Operators & punctuation
;; ============================================================

;; #F29668 #000000 0 0 0 0 2
(quantifier) @operator

;; #BFBDB6 #000000 0 0 0 0 2
(predicate_type) @punctuation.special

;; #F29668 #000000 0 0 0 0 2
"." @operator

;; #BFBDB6 #000000 0 0 0 0 2
[
  "["
  "]"
  "("
  ")"
] @punctuation.bracket

;; #BFBDB6 #000000 0 0 0 0 2
[
  ":"
  "/"
] @punctuation.delimiter

;; #BFBDB6 #000000 0 0 0 0 2
[
  "@"
  "#"
] @punctuation.special

;; #BFBDB6 #000000 0 0 0 0 2
(predicate
  "." @punctuation.special)

;; #D2A6FF #000000 0 0 0 0 2
"_" @character.special

;; #FF8F40 #000000 0 0 0 0 2
"MISSING" @keyword

;; ============================================================
;; Numbers
;; ============================================================

;; #B8E986 #000000 0 0 0 0 2
((parameters
  (identifier) @number)
  (#match? @number "^[-+]?[0-9]+(.[0-9]+)?$"))

;; ============================================================
;; Predicate parameters
;; ============================================================

;; #F29CC3 #000000 0 0 0 0 2
((predicate
  name: (identifier) @_name
  parameters: (parameters
    .
    (capture)?
    .
    (identifier) @property))
  (#match? @_name "^set$"))

;; #AAD94C #000000 0 0 0 0 2
((predicate
  name: (identifier) @_name
  parameters: (parameters
    (string
      "\"" @string
      "\"" @string)
;; !regex
    @string.regexp))
  (#match? @_name "^(match|not-match)$"))

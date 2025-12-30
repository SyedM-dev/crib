;; #D2A6FF #000000 0 0 0 0 1
(tag_name) @tag
(nesting_selector) @tag
(universal_selector) @tag

;; #F29668 #000000 0 0 0 0 1
"~" @operator
">" @operator
"+" @operator
"-" @operator
"*" @operator
"/" @operator
"=" @operator
"^=" @operator
"|=" @operator
"~=" @operator
"$=" @operator
"*=" @operator
"and" @operator
"or" @operator
"not" @operator
"only" @operator

;; #AAD94C #000000 0 0 0 0 2
(attribute_selector (plain_value) @string)
(string_value) @string

;; #FFFFFF #000000 0 0 0 0 1
((property_name) @variable
 (#match? @variable "^--"))
((plain_value) @variable
 (#match? @variable "^--"))

;; #7dcfff #000000 0 0 0 0 2
(class_name) @property
(id_name) @property
(namespace_name) @property
(property_name) @property
(feature_name) @property

;; #F07178 #000000 0 0 0 0 1
(pseudo_element_selector (tag_name) @attribute)
(pseudo_class_selector (class_name) @attribute)
(attribute_name) @attribute

;; #F07178 #000000 0 0 0 0 1
(function_name) @function

;; #99ADBF #000000 0 1 0 0 1
(comment) @comment

;; #FFFFFF #000000 0 0 0 0 1
(color_value) @string.special

;; #FF8F40 #000000 0 0 0 0 1
(integer_value) @number
(float_value) @number

;; #FF8F40 #000000 0 0 0 0 1
(unit) @type

;; #AAD94C #000000 0 0 0 0 3
[
  "#"
  ","
  "."
  ":"
  "::"
  ";"
] @punctuation.delimiter

;; #AAD94C #000000 0 0 0 0 3
[
  "{"
  "}"
  ")"
  "("
  "["
  "]"
] @punctuation.bracket

;; #D2A6FF #000000 0 0 0 0 1
(at_keyword) @keyword
(to) @keyword
(from) @keyword
(important) @keyword

; This is put at the end as the regex parser will wrongly think @media is a capture name becouse of its @
; TODO: This should be fixed by not selecting if it is in a string
"@media" @keyword
"@import" @keyword
"@charset" @keyword
"@namespace" @keyword
"@supports" @keyword
"@keyframes" @keyword

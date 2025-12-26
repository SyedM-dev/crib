; ============================================================
; Identifiers & Modules
; ============================================================

;; #82AAFF #000000 0 0 0 1
(shebang) @keyword.directive1

;; #E5C07B #000000 0 0 0 1
(identifier) @variable1

;; #A6E22E #000000 0 0 0 2
((identifier) @type1
  (#match? @type1 "^[A-Z]"))

;; #FFD700 #000000 0 0 0 2
(const_item
  name: (identifier) @constant1)

;; #FF9E64 #000000 0 0 0 3
((identifier) @constant2
  (#match? @constant2 "^[A-Z][A-Z%d_]*$"))

;; #7DCFFF #000000 0 0 0 4
(type_identifier) @type2

;; #7DCFFF #000000 0 0 0 2
(primitive_type) @type.builtin1

;; #C678DD #000000 0 0 0 2
(field_identifier) @variable.member1

;; #C678DD #000000 0 0 0 2
(shorthand_field_identifier) @variable.member2

;; #C678DD #000000 0 0 0 2
(shorthand_field_initializer
  (identifier) @variable.member3)

;; #61AFEF #000000 0 0 0 2
(mod_item
  name: (identifier) @module1)

;; #D19A66 #000000 0 0 0 2
(self) @variable.builtin1

;; #5C6370 #000000 0 0 0 1
"_" @character.special1

;; #61AFEF #000000 0 0 1 2
(label
  [
    "'"
    (identifier)
  ] @label1)

; ============================================================
; Functions & Parameters
; ============================================================

;; #FFB454 #000000 0 0 0 3
(function_item
  (identifier) @function1)

;; #FFB454 #000000 0 0 0 3
(function_signature_item
  (identifier) @function2)

;; #D2A6FF #000000 0 0 0 1
(parameter
  [
    (identifier)
    "_"
  ] @variable.parameter1)

;; #D2A6FF #000000 0 0 0 1
(parameter
  (ref_pattern
    [
      (mut_pattern
        (identifier) @variable.parameter2)
      (identifier) @variable.parameter3
    ]))

;; #D2A6FF #000000 0 0 0 1
(closure_parameters
  (_) @variable.parameter4)

; ============================================================
; Function Calls & Constructors
; ============================================================

;; #FFB454 #000000 0 0 0 2
(call_expression
  function: (identifier) @function.call1)

;; #FFB454 #000000 0 0 0 2
(call_expression
  function: (scoped_identifier
    (identifier) @function.call2 .))

;; #FFB454 #000000 0 0 0 2
(call_expression
  function: (field_expression
    field: (field_identifier) @function.call3))

;; #FFB454 #000000 0 0 0 2
(generic_function
  function: (identifier) @function.call4)

;; #FFB454 #000000 0 0 0 2
(generic_function
  function: (scoped_identifier
    name: (identifier) @function.call5))

;; #FFB454 #000000 0 0 0 2
(generic_function
  function: (field_expression
    field: (field_identifier) @function.call6))

;; #9ADE7A #000000 0 0 0 32
((field_identifier) @constant3
  (#match? @constant3 "^[A-Z]"))

;; #9ADE7A #000000 0 0 0 32
(enum_variant
  name: (identifier) @constant4)

; ============================================================
; Scoped Identifiers & Paths
; ============================================================

;; #82AAFF #000000 0 0 0 9
(scoped_identifier
  path: (identifier) @module2)

;; #82AAFF #000000 0 0 0 9
(scoped_identifier
  (scoped_identifier
    name: (identifier) @module3))

;; #7DCFFF #000000 0 0 0 9
(scoped_type_identifier
  path: (identifier) @module4)

;; #7DCFFF #000000 0 0 0 9
(scoped_type_identifier
  path: (identifier) @type3
  (#match? @type3 "^[A-Z]"))

;; #7DCFFF #000000 0 0 0 9
(scoped_type_identifier
  (scoped_identifier
    name: (identifier) @module5))

;; #7DCFFF #000000 0 0 0 9
((scoped_identifier
  path: (identifier) @type4)
  (#match? @type4 "^[A-Z]"))

;; #7DCFFF #000000 0 0 0 9
((scoped_identifier
  name: (identifier) @type5)
  (#match? @type5 "^[A-Z]"))

;; #FFD700 #000000 0 0 0 7
((scoped_identifier
  name: (identifier) @constant5)
  (#match? @constant5 "^[A-Z][A-Z%d_]*$"))

;; #FFD700 #000000 0 0 0 7
((scoped_identifier
  path: (identifier) @type6
  name: (identifier) @constant6)
  (#match? @type6 "^[A-Z]")
  (#match? @constant6 "^[A-Z]"))

;; #FFD700 #000000 0 0 0 7
((scoped_type_identifier
  path: (identifier) @type7
  name: (type_identifier) @constant7)
  (#match? @type7 "^[A-Z]")
  (#match? @constant7 "^[A-Z]"))

;; #61AFEF #000000 0 0 0 0
[
  (crate)
  (super)
] @module6

;; #61AFEF #000000 0 0 0 0
(scoped_use_list
  path: (identifier) @module7)

;; #61AFEF #000000 0 0 0 0
(scoped_use_list
  path: (scoped_identifier
    (identifier) @module8))

;; #7DCFFF #000000 0 0 0 0
(use_list
  (scoped_identifier
    (identifier) @module9
    .
    (_)))

;; #7DCFFF #000000 0 0 0 0
(use_list
  (identifier) @type8
  (#match? @type8 "^[A-Z]"))

;; #7DCFFF #000000 0 0 0 0
(use_as_clause
  alias: (identifier) @type9
  (#match? @type9 "^[A-Z]"))

; ============================================================
; Enum Constructors & Match Arms
; ============================================================

;; #9ADE7A #000000 0 0 0 9
; Correct enum constructors
(call_expression
  function: (scoped_identifier
    "::"
    name: (identifier) @constant8)
  (#match? @constant8 "^[A-Z]"))

;; #FFD700 #000000 0 0 0 2
; Assume uppercase names in a match arm are constants.
((match_arm
  pattern: (match_pattern
    (identifier) @constant9))
  (#match? @constant9 "^[A-Z]"))

;; #FFD700 #000000 0 0 0 2
((match_arm
  pattern: (match_pattern
    (scoped_identifier
      name: (identifier) @constant10)))
  (#match? @constant10 "^[A-Z]"))

;; #D2A6FF #000000 0 0 0 3
((identifier) @constant.builtin1
  (#match? @constant.builtin1 "^(Some|None|Ok|Err)$"))

; ============================================================
; Macros
; ============================================================

;; #FF8F40 #000000 0 0 0 2
"$" @function.macro1

;; #FF8F40 #000000 0 0 0 2
(metavariable) @function.macro2

;; #FF8F40 #000000 0 0 0 2
(macro_definition
  "macro_rules!" @function.macro3)

;; #FF8F40 #000000 0 0 0 2
(attribute_item
  (attribute
    (identifier) @function.macro4))

;; #FF8F40 #000000 0 0 0 2
(inner_attribute_item
  (attribute
    (identifier) @function.macro5))

;; #FF8F40 #000000 0 0 0 2
(attribute
  (scoped_identifier
    (identifier) @function.macro6 .))

;; #FF8F40 #000000 0 0 0 2
(macro_invocation
  macro: (identifier) @function.macro7)

;; #FF8F40 #000000 0 0 0 2
(macro_invocation
  macro: (scoped_identifier
    (identifier) @function.macro8 .))

; ============================================================
; Literals
; ============================================================

;; #D2A6FF #000000 0 0 0 1
(boolean_literal) @boolean1

;; #D2A6FF #000000 0 0 0 1
(integer_literal) @number1

;; #D2A6FF #000000 0 0 0 1
(float_literal) @number.float1

;; #AAD94C #000000 0 0 0 0
[
  (raw_string_literal)
  (string_literal)
] @string1

;; #AAD94C #000000 0 0 0 0
(escape_sequence) @string.escape1

;; #F07178 #000000 0 0 0 1
(char_literal) @character1

; ============================================================
; Keywords
; ============================================================

;; #FF8F40 #000000 0 0 0 1
[
  "use"
  "mod"
] @keyword.import1

;; #FF8F40 #000000 0 0 0 1
(use_as_clause
  "as" @keyword.import2)

;; #FF8F40 #000000 0 0 0 1
[
  "default"
  "impl"
  "let"
  "move"
  "unsafe"
  "where"
] @keyword1

;; #FF8F40 #000000 0 0 0 1
[
  "enum"
  "struct"
  "union"
  "trait"
  "type"
] @keyword.type1

;; #82AAFF #000000 0 0 0 1
[
  "async"
  "await"
  "gen"
] @keyword.coroutine1

;; #FF6347 #000000 0 0 0 1
"try" @keyword.exception1

;; #FF8F40 #000000 0 0 0 1
[
  "ref"
  "pub"
  "raw"
  (mutable_specifier)
  "const"
  "static"
  "dyn"
  "extern"
] @keyword.modifier1

;; #FF8F40 #000000 0 0 0 1
(lifetime
  "'" @keyword.modifier2)

;; #9ADE7A #000000 0 0 0 5
(lifetime
  (identifier) @attribute1)

;; #9ADE7A #000000 0 0 0 5
(lifetime
  (identifier) @attribute.builtin1
  (#match? @attribute.builtin1 "^(static|_)$"))

;; #FF8F40 #000000 0 0 0 1
"fn" @keyword.function1

;; #FF8F40 #000000 0 0 0 1
[
  "return"
  "yield"
] @keyword.return1

;; #F29668 #000000 0 0 0 1
(type_cast_expression
  "as" @keyword.operator1)

;; #F29668 #000000 0 0 0 1
(qualified_type
  "as" @keyword.operator2)

;; #61AFEF #000000 0 0 0 9
(use_list
  (self) @module10)

;; #61AFEF #000000 0 0 0 9
(scoped_use_list
  (self) @module11)

;; #61AFEF #000000 0 0 0 9
(scoped_identifier
  [
    (crate)
    (super)
    (self)
  ] @module12)

;; #61AFEF #000000 0 0 0 9
(visibility_modifier
  [
    (crate)
    (super)
    (self)
  ] @module13)

;; #FF8F40 #000000 0 0 0 1
[
  "if"
  "else"
  "match"
] @keyword.conditional1

;; #FF8F40 #000000 0 0 0 1
[
  "break"
  "continue"
  "in"
  "loop"
  "while"
] @keyword.repeat1

;; #FF8F40 #000000 0 0 0 1
"for" @keyword2

;; #FF8F40 #000000 0 0 0 1
(for_expression
  "for" @keyword.repeat2)

; ============================================================
; Operators
; ============================================================

;; #F29668 #000000 0 1 0 1
[
  "!"
  "!="
  "%"
  "%="
  "&"
  "&&"
  "&="
  "*"
  "*="
  "+"
  "+="
  "-"
  "-="
  ".."
  "..="
  "..."
  "/"
  "/="
  "<"
  "<<"
  "<<="
  "<="
  "="
  "=="
  ">"
  ">="
  ">>"
  ">>="
  "?"
  "@"
  "^"
  "^="
  "|"
  "|="
  "||"
] @operator1

;; #BFBDB6 #000000 0 0 0 1
(use_wildcard
  "*" @character.special2)

;; #BFBDB6 #000000 0 0 0 1
(remaining_field_pattern
  ".." @character.special3)

;; #BFBDB6 #000000 0 0 0 1
(range_pattern
  [
    ".."
    "..="
    "..."
  ] @character.special4)

; ============================================================
; Punctuation
; ============================================================

;; #BFBDB6 #000000 0 0 0 1
[
  "("
  ")"
  "["
  "]"
  "{"
  "}"
] @punctuation.bracket1

;; #BFBDB6 #000000 0 0 0 1
(closure_parameters
  "|" @punctuation.bracket2)

;; #BFBDB6 #000000 0 0 0 1
(type_arguments
  [
    "<"
    ">"
  ] @punctuation.bracket3)

;; #BFBDB6 #000000 0 0 0 1
(type_parameters
  [
    "<"
    ">"
  ] @punctuation.bracket4)

;; #BFBDB6 #000000 0 0 0 1
(bracketed_type
  [
    "<"
    ">"
  ] @punctuation.bracket5)

;; #BFBDB6 #000000 0 0 0 1
(for_lifetimes
  [
    "<"
    ">"
  ] @punctuation.bracket6)

;; #BFBDB6 #000000 0 1 0 1
[
  ","
  "."
  ":"
  "::"
  ";"
  "->"
  "=>"
] @punctuation.delimiter1

;; #BFBDB6 #000000 0 0 0 1
(attribute_item
  "#" @punctuation.special1)

;; #BFBDB6 #000000 0 0 0 1
(inner_attribute_item
  [
    "!"
    "#"
  ] @punctuation.special2)

;; #FF8F40 #000000 0 0 0 2
(macro_invocation
  "!" @function.macro9)

;; #7DCFFF #000000 0 0 0 1
(never_type
  "!" @type.builtin2)

; ============================================================
; Panic / Assert / Debug Macros
; ============================================================

;; #FF6347 #000000 0 0 0 2
(macro_invocation
  macro: (identifier) @_identifier1 @keyword.exception2
  "!" @keyword.exception2
  (#match? @_identifier1 "^panic$"))

;; #FF8F40 #000000 0 0 0 2
(macro_invocation
  macro: (identifier) @_identifier2 @keyword.exception3
  "!" @keyword.exception3
  (#match? @_identifier2 "assert"))

;; #7DCFFF #000000 0 0 0 2
(macro_invocation
  macro: (identifier) @_identifier3 @keyword.debug1
  "!" @keyword.debug1
  (#match? @_identifier3 "^dbg$"))

; ============================================================
; Comments
; ============================================================

;; #99ADBF #000000 0 1 0 1
[
  (line_comment)
  (block_comment)
  (outer_doc_comment_marker)
  (inner_doc_comment_marker)
] @comment1

(line_comment
  (doc_comment)) @comment2

(block_comment
  (doc_comment)) @comment3

; ============================================================
; Regex Strings (highlighted)
; ============================================================

(call_expression
  function: (scoped_identifier
    path: (identifier) @_regex1
    (#match? @_regex1 "Regex")
    name: (identifier) @_new1
    (#match? @_new1 "^new$"))
  arguments: (arguments
    (raw_string_literal
;; !regex
      (string_content) @string.regexp)))

(call_expression
  function: (scoped_identifier
    path: (scoped_identifier
      (identifier) @_regex2
      (#match? @_regex2 "Regex") .)
    name: (identifier) @_new2
    (#match? @_new2 "^new$"))
  arguments: (arguments
    (raw_string_literal
      (string_content) @string.regexp)))

(call_expression
  function: (scoped_identifier
    path: (identifier) @_regex3
    (#match? @_regex3 "Regex")
    name: (identifier) @_new3
    (#match? @_new3 "^new$"))
  arguments: (arguments
    (array_expression
      (raw_string_literal
        (string_content) @string.regexp))))

(call_expression
  function: (scoped_identifier
    path: (scoped_identifier
      (identifier) @_regex4
      (#match? @_regex4 "Regex") .)
    name: (identifier) @_new4
    (#match? @_new4 "^new$"))
  arguments: (arguments
    (array_expression
      (raw_string_literal
        (string_content) @string.regexp))))

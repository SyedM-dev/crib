; ============================================================
; Identifiers
; ============================================================

;; #FFFFFF #000000 0 0 0 0 1
(identifier) @variable

;; #C9B4FF #000000 0 0 0 0 2
((identifier) @constant
 (#match? @constant "^[A-Z][A-Z_0-9]*$"))

;; #F28FAD #000000 0 0 0 0 3
((identifier) @variable.builtin
 (#match? @variable.builtin "^self$"))

; Attributes (generic parameters)
;; #7CD5CF #000000 0 0 0 0 2
(variable_list
  (attribute
    "<" @punctuation.bracket
    (identifier) @attribute
    ">" @punctuation.bracket))

; ============================================================
; Control flow & keywords
; ============================================================

;; #9AD4FF #000000 0 0 0 0 2
"return" @keyword.return

;; #FF9E64 #000000 0 0 0 0 2
[
 "goto"
 "in"
 "local"
] @keyword

;; #9AD4FF #000000 0 0 0 0 2
(label_statement) @label

;; #FF9E64 #000000 0 0 0 0 2
(break_statement) @keyword

;; #9AD4FF #000000 0 0 0 0 2
(do_statement
[
  "do"
  "end"
] @keyword)

;; #9AD4FF #000000 0 0 0 0 2
(while_statement
[
  "while"
  "do"
  "end"
] @repeat)

;; #9AD4FF #000000 0 0 0 0 2
(repeat_statement
[
  "repeat"
  "until"
] @repeat)

;; #FFB870 #000000 0 0 0 0 2
(if_statement
[
  "if"
  "elseif"
  "else"
  "then"
  "end"
] @conditional)

;; #9AD4FF #000000 0 0 0 0 2
(elseif_statement
[
  "elseif"
  "then"
  "end"
] @conditional)

;; #9AD4FF #000000 0 0 0 0 2
(else_statement
[
  "else"
  "end"
] @conditional)

;; #9AD4FF #000000 0 0 0 0 2
(for_statement
[
  "for"
  "do"
  "end"
] @repeat)

;; #FFB870 #000000 0 0 0 0 2
(function_declaration
[
  "function"
  "end"
] @keyword.function)

;; #FFB870 #000000 0 0 0 0 2
(function_definition
[
  "function"
  "end"
] @keyword.function)

; ============================================================
; Operators
; ============================================================

;; #6BD9DF #000000 0 1 0 0 1
(binary_expression operator: _ @operator)

;; #6BD9DF #000000 0 1 0 0 1
(unary_expression operator: _ @operator)

;; #F29CC3 #000000 0 0 0 0 1
[
 "and"
 "not"
 "or"
] @keyword.operator

; ============================================================
; Punctuation
; ============================================================

;; #B6BEC8 #000000 0 0 0 0 1
[
 ";"
 ":"
 ","
 "."
] @punctuation.delimiter

; Brackets
;; #B6BEC8 #000000 0 0 0 0 1
[
 "("
 ")"
 "["
 "]"
 "{"
 "}"
] @punctuation.bracket

; ============================================================
; Tables & fields
; ============================================================

;; #9AD4FF #000000 0 0 0 0 1
(field name: (identifier) @field)

;; #9AD4FF #000000 0 0 0 0 1
(dot_index_expression field: (identifier) @field)

;; #7CD5CF #000000 0 0 0 0 1
(table_constructor
[
  "{"
  "}"
] @constructor)

; ============================================================
; Functions
; ============================================================

;; #FFC877 #000000 0 0 0 0 3
(parameters (identifier) @parameter)

;; #FFC877 #000000 0 0 0 0 3
(function_declaration
  name: [
    (identifier) @function
    (dot_index_expression
      field: (identifier) @function)
  ])

;; #FFC877 #000000 0 0 0 0 3
(function_declaration
  name: (method_index_expression
    method: (identifier) @method))

;; #FFC877 #000000 0 0 0 0 3
(assignment_statement
  (variable_list .
    name: [
      (identifier) @function
      (dot_index_expression
        field: (identifier) @function)
    ])
  (expression_list .
    value: (function_definition)))

;; #FFC877 #000000 0 0 0 0 3
(table_constructor
  (field
    name: (identifier) @function
    value: (function_definition)))

; Function calls
;; #78C2FF #000000 0 0 0 0 2
(function_call
  name: [
    (identifier) @function.call
    (dot_index_expression
      field: (identifier) @function.call)
    (method_index_expression
      method: (identifier) @method.call)
  ])

; Highlighted definitions & references
;; #FFC877 #000000 0 0 0 0 3
(function_declaration
  name: [
    (identifier) @name
    (dot_index_expression
      field: (identifier) @name)
  ]) @definition.function

;; #FFC877 #000000 0 0 0 0 3
(function_declaration
  name: (method_index_expression
    method: (identifier) @name)) @definition.method

;; #FFC877 #000000 0 0 0 0 3
(assignment_statement
  (variable_list .
    name: [
      (identifier) @name
      (dot_index_expression
        field: (identifier) @name)
    ])
  (expression_list .
    value: (function_definition))) @definition.function

;; #FFC877 #000000 0 0 0 0 3
(table_constructor
  (field
    name: (identifier) @name
    value: (function_definition))) @definition.function

;; #78C2FF #000000 0 0 0 0 2
(function_call
  name: [
    (identifier) @name
    (dot_index_expression
      field: (identifier) @name)
    (method_index_expression
      method: (identifier) @name)
  ]) @reference.call

; Builtins
;; #F28FAD #000000 0 0 0 0 2
(function_call
  (identifier) @function.builtin
  (#match? @function.builtin "^(assert|collectgarbage|dofile|error|getfenv|getmetatable|ipairs|load|loadfile|loadstring|module|next|pairs|pcall|print|rawequal|rawget|rawset|required|select|setfenv|setmetatable|tonumber|tostring|type|unpack|xpcall)$"))

; ============================================================
; Literals & constants
; ============================================================

;; #B8E986 #000000 0 0 0 0 5
(number) @number

;; #A6E3A1 #000000 0 0 0 0 5
(string) @string

;; #A6E3A1 #000000 0 0 0 0 6
(escape_sequence) @string.escape

;; #C9B4FF #000000 0 0 0 0 2
(vararg_expression) @constant

;; #C9B4FF #000000 0 0 0 0 2
(nil) @constant.builtin

;; #C2E8FF #000000 0 0 0 0 2
[
  (false)
  (true)
] @boolean

; ============================================================
; Comments & directives
; ============================================================

;; #99ADBF #000000 0 1 0 0 1
(comment) @comment

;; #7CD5CF #000000 0 0 0 0 1
(hash_bang_line) @preproc

; ============================================================
; Injections
; ============================================================

;; #7CD5CF #000000 0 1 0 0 2
((function_call
  name: [
    (identifier) @_cdef_identifier
    (_ _ (identifier) @_cdef_identifier)
  ]
  ;; !c
  arguments: (arguments (string content: _ @injection.c)))
  (#match? @_cdef_identifier "^cdef$"))

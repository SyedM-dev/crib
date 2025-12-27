; ============================================================
; Identifiers
; ============================================================

;; #FFFFFF #000000 0 0 0 1
(identifier) @variable

;; #D2A6FF #000000 0 0 0 2
((identifier) @constant
 (#match? @constant "^[A-Z_][A-Z0-9_]*$"))

;; #F07178 #000000 0 0 0 3
((identifier) @variable.builtin
 (#match? @variable.builtin
  "^(arguments|console|window|document|globalThis|process|module|exports)$"))

;; #59C2FF #000000 0 0 0 1
((identifier) @constructor
 (#match? @constructor "^[A-Z][a-zA-Z0-9]*$"))

; ============================================================
; Properties
; ============================================================

;; #F07178 #000000 0 0 0 1
(property_identifier) @property

; ============================================================
; Functions
; ============================================================

;; #FFB454 #000000 0 0 0 3
(function_declaration
  name: (identifier) @function)

(function_expression
  name: (identifier) @function)

;; #FFB454 #000000 0 0 0 2
(method_definition
  name: (property_identifier) @function.method)

(variable_declarator
  name: (identifier) @function
  value: [(function_expression) (arrow_function)])

(assignment_expression
  left: (identifier) @function
  right: [(function_expression) (arrow_function)])

(pair
  key: (property_identifier) @function.method
  value: [(function_expression) (arrow_function)])

; ------------------------------------------------------------
; Function calls
; ------------------------------------------------------------

;; #FFB454 #000000 0 0 0 2
(call_expression
  function: (identifier) @function.call)

;; #FFB454 #000000 0 0 0 2
(call_expression
  function: (member_expression
    property: (property_identifier) @function.method))

; ============================================================
; Highlighted definitions & references
; ============================================================

;; #FFB454 #000000 0 0 0 3
(assignment_expression
  left: [
    (identifier) @name
    (member_expression
      property: (property_identifier) @name)
  ]
  right: [(arrow_function) (function_expression)]
) @definition.function

;; #FFB454 #000000 0 0 0 3
(pair
  key: (property_identifier) @name
  value: [(arrow_function) (function_expression)]) @definition.function

;; #59C2FF #000000 0 0 0 0
(
  (call_expression
    function: (identifier) @name) @reference.call
  (#not-match? @name "^(require)$"))

;; #7dcfff #000000 0 0 0 2
(new_expression
  constructor: (_) @name) @reference.class

;; #D2A6FF #000000 0 0 0 2
(export_statement value: (assignment_expression left: (identifier) @name right: ([
 (number)
 (string)
 (identifier)
 (undefined)
 (null)
 (new_expression)
 (binary_expression)
 (call_expression)
]))) @definition.constant

; ============================================================
; Parameters
; ============================================================

;; #D2A6FF #000000 0 0 0 1
(formal_parameters
  [
    (identifier) @variable.parameter
    (array_pattern
      (identifier) @variable.parameter)
    (object_pattern
      [
        (pair_pattern value: (identifier) @variable.parameter)
        (shorthand_property_identifier_pattern) @variable.parameter
      ])
  ])

; ============================================================
; Keywords (split into semantic groups)
; ============================================================

;; #FF8F40 #000000 0 0 0 1
; Declarations
[
  "var"
  "let"
  "const"
  "function"
  "class"
] @keyword.declaration

;; #FF8F40 #000000 0 0 0 1
; Control flow
[
  "if"
  "else"
  "switch"
  "case"
  "default"
  "for"
  "while"
  "do"
  "break"
  "continue"
  "return"
  "throw"
  "try"
  "catch"
  "finally"
  "extends"
] @keyword.control

;; #FF8F40 #000000 0 0 0 1
; Imports / exports
[
  "import"
  "export"
  "from"
  "as"
] @keyword.import

;; #F29668 #000000 0 0 0 1
; Operators-as-keywords
[
  "in"
  "instanceof"
  "new"
  "delete"
  "typeof"
  "void"
  "await"
  "yield"
] @keyword.operator

;; #FF8F40 #000000 0 0 0 1
; Modifiers
[
  "async"
  "static"
  "get"
  "set"
] @keyword.modifier

; ============================================================
; Literals
; ============================================================

;; #F07178 #000000 0 0 0 1
(this) @variable.builtin
(super) @variable.builtin

;; #D2A6FF #000000 0 0 0 4
[
  (true)
  (false)
  (null)
  (undefined)
] @constant.builtin

;; #D2A6FF #000000 0 0 0 2
(number) @number

;; #D2A6FF #000000 0 1 0 2
((string) @use_strict
  (#match? @use_strict "^['\"]use strict['\"]$"))

;; #AAD94C #000000 0 0 0 0
(string) @string

;; #AAD94C #000000 0 0 0 0
(template_string) @string.special

;; #99ADBF #000000 0 1 0 1
(comment) @comment

; ============================================================
; Operators & punctuation
; ============================================================

;; #F29668 #000000 0 1 0 1
[
  "+"
  "-"
  "*"
  "/"
  "%"
  "**"
  "++"
  "--"
  "=="
  "!="
  "==="
  "!=="
  "<"
  "<="
  ">"
  ">="
  "&&"
  "||"
  "??"
  "!"
  "~"
  "&"
  "|"
  "^"
  "<<"
  ">>"
  ">>>"
  "="
  "+="
  "-="
  "*="
  "/="
  "%="
  "<<="
  ">>="
  ">>>="
  "&="
  "|="
  "^="
  "&&="
  "||="
  "??="
  "=>"
] @operator

;; #BFBDB6 #000000 0 0 0 1
[
  "."
  ","
  ";"
] @punctuation.delimiter

;; #BFBDB6 #000000 0 0 0 1
[
  "("
  ")"
  "["
  "]"
  "{"
  "}"
] @punctuation.bracket

;; #7dcfff #000000 0 0 0 2
(template_substitution
  "${" @punctuation.special
  "}" @punctuation.special)

; ============================================================
; JSX
; ============================================================

;; #59C2FF #000000 0 0 0 4
(jsx_opening_element (identifier) @tag2)
(jsx_closing_element (identifier) @tag2)
(jsx_self_closing_element (identifier) @tag2)

;; #F07178 #000000 0 0 0 3
(jsx_attribute (property_identifier) @attribute2)

;; #BFBDB6 #000000 0 0 0 3
(jsx_opening_element (["<" ">"]) @punctuation.bracket2)
(jsx_closing_element (["</" ">"]) @punctuation.bracket2)
(jsx_self_closing_element (["<" "/>"]) @punctuation.bracket2)

; Injections

;; !regex
(regex) @string.regex

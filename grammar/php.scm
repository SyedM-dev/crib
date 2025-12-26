;; =========================================================
;; PHP SYNTAX HIGHLIGHTING
;; Cool–warm balanced palette (blue / teal / purple / orange)
;; =========================================================

;; #FF9D5C #000000 0 0 0 1
;; Keywords (logic / flow)
[
  "and"
  "as"
  "instanceof"
  "or"
  "xor"
] @keyword.operator

;; #FF9D5C #000000 0 0 0 1
[
  "fn"
  "function"
] @keyword.function

;; #FF9D5C #000000 0 0 0 1
[
  "clone"
  "declare"
  "default"
  "echo"
  "enddeclare"
  "extends"
  "global"
  "goto"
  "implements"
  "insteadof"
  "print"
  "new"
  "unset"
] @keyword

;; #6FB3FF #000000 0 0 0 1
[
  "enum"
  "class"
  "interface"
  "namespace"
  "trait"
] @keyword.type

;; #FF9D5C #000000 0 0 0 1
[
  "abstract"
  "const"
  "final"
  "private"
  "protected"
  "public"
  "readonly"
  "static"
] @keyword.modifier

;; #FF9D5C #000000 0 0 0 1
[
  "return"
  "exit"
  "yield"
  "yield from"
] @keyword.return

;; #FF9D5C #000000 0 0 0 1
[
  "case"
  "else"
  "elseif"
  "endif"
  "endswitch"
  "if"
  "switch"
  "match"
  "??"
] @keyword.conditional

;; #FF9D5C #000000 0 0 0 1
[
  "break"
  "continue"
  "do"
  "endfor"
  "endforeach"
  "endwhile"
  "for"
  "foreach"
  "while"
] @keyword.repeat

;; #FF9D5C #000000 0 0 0 1
[
  "catch"
  "finally"
  "throw"
  "try"
] @keyword.exception

;; #8BD5CA #000000 0 0 0 1
[
  "include_once"
  "include"
  "require_once"
  "require"
  "use"
] @keyword.import

;; #B0BEC5 #000000 0 0 0 1
[
  ","
  ";"
  ":"
  "\\"
] @punctuation.delimiter

;; #B0BEC5 #000000 0 0 0 1
[
  (php_tag)
  (php_end_tag)
  "("
  ")"
  "["
  "]"
  "{"
  "}"
  "#["
] @punctuation.bracket

;; #F29668 #000000 0 1 0 1
[
  "="
  "."
  "-"
  "*"
  "/"
  "+"
  "%"
  "**"
  "~"
  "|"
  "^"
  "&"
  "<<"
  ">>"
  "<<<"
  "->"
  "?->"
  "=>"
  "<"
  "<="
  ">="
  ">"
  "<>"
  "<=>"
  "=="
  "!="
  "==="
  "!=="
  "!"
  "&&"
  "||"
  ".="
  "-="
  "+="
  "*="
  "/="
  "%="
  "**="
  "&="
  "|="
  "^="
  "<<="
  ">>="
  "??="
  "--"
  "++"
  "@"
  "::"
] @operator

;; #7DCFFF #000000 0 0 0 1
(variable_name) @variable

;; #C792EA #000000 0 0 0 1
((name) @constant
  (#lua-match? @constant "^_?[A-Z][A-Z%d_]*$"))

;; #C792EA #000000 0 0 0 1
((name) @constant.builtin
  (#lua-match? @constant.builtin "^__[A-Z][A-Z%d_]+__$"))

;; #6FB3FF #000000 0 0 0 1
(const_declaration
  (const_element
    (name) @constant))

;; #82AAFF #000000 0 0 0 1
[
  (primitive_type)
  (cast_type)
  (bottom_type)
] @type.builtin

;; #82AAFF #000000 0 0 0 1
(named_type
  [
    (name) @type
    (qualified_name (name) @type)
    (relative_name (name) @type)
  ])

;; #82AAFF #000000 0 0 0 1
(named_type
  (name) @type.builtin
  (#any-of? @type.builtin "static" "self"))

;; #82AAFF #000000 0 0 0 1
(class_declaration
  name: (name) @type)

;; #82AAFF #000000 0 0 0 1
(enum_declaration
  name: (name) @type)

;; #82AAFF #000000 0 0 0 1
(interface_declaration
  name: (name) @type)

;; #7DCFFF #000000 0 0 0 1
(namespace_use_clause
  [
    (name) @type
    (qualified_name (name) @type)
    alias: (name) @type.definition
  ])

;; #7DCFFF #000000 0 0 0 1
(namespace_use_clause
  type: "function"
  [
    (name) @function
    (qualified_name (name) @function)
    alias: (name) @function
  ])

;; #7DCFFF #000000 0 0 0 1
(namespace_use_clause
  type: "const"
  [
    (name) @constant
    (qualified_name (name) @constant)
    alias: (name) @constant
  ])

;; #7DCFFF #000000 0 0 0 1
(scoped_call_expression
  scope: [
    (name) @type
    (qualified_name (name) @type)
    (relative_name (name) @type)
  ])

;; #7DCFFF #000000 0 0 0 1
(class_constant_access_expression
  .
  [
    (name) @type
    (qualified_name (name) @type)
    (relative_name (name) @type)
  ]
  (name) @constant)

;; #A6E3A1 #000000 0 0 0 1
(scoped_property_access_expression
  name: (variable_name) @variable.member)

;; #A6E3A1 #000000 0 0 0 1
(trait_declaration
  name: (name) @type)

;; #A6E3A1 #000000 0 0 0 1
(use_declaration
  (name) @type)

;; #FF9D5C #000000 0 0 0 1
(binary_expression
  operator: "instanceof"
  right: [
    (name) @type
    (qualified_name (name) @type)
    (relative_name (name) @type)
  ])

;; #FFD580 #000000 0 0 0 1
(array_creation_expression
  "array" @function.builtin)

;; #FFD580 #000000 0 0 0 1
(list_literal
  "list" @function.builtin)

;; #FFD580 #000000 0 0 0 1
(exit_statement
  "exit" @function.builtin
  "(")

;; #89DDFF #000000 0 0 0 1
(method_declaration
  name: (name) @function.method)

;; #89DDFF #000000 0 0 0 1
(function_call_expression
  function: [
    (name) @function.call
    (qualified_name (name) @function.call)
    (relative_name (name) @function.call)
  ])

;; #89DDFF #000000 0 0 0 1
(scoped_call_expression
  name: (name) @function.call)

;; #89DDFF #000000 0 0 0 1
(member_call_expression
  name: (name) @function.method)

;; #89DDFF #000000 0 0 0 1
(nullsafe_member_call_expression
  name: (name) @function.method)

;; #FFD580 #000000 0 0 0 1
(method_declaration
  name: (name) @constructor
  (#eq? @constructor "__construct"))

;; #FFD580 #000000 0 0 0 1
(object_creation_expression
  [
    (name) @constructor
    (qualified_name (name) @constructor)
    (relative_name (name) @constructor)
  ])

;; #9CDCFE #000000 0 0 0 1
(variadic_parameter
  "..." @operator
  name: (variable_name) @variable.parameter)

;; #9CDCFE #000000 0 0 0 1
(simple_parameter
  name: (variable_name) @variable.parameter)

;; #9CDCFE #000000 0 0 0 1
(argument
  (name) @variable.parameter)

;; #9CDCFE #000000 0 0 0 1
(property_element
  (variable_name) @property)

;; #9CDCFE #000000 0 0 0 1
(member_access_expression
  name: (variable_name (name)) @variable.member)

;; #9CDCFE #000000 0 0 0 1
(relative_scope) @variable.builtin

;; #7AA2F7 #000000 0 0 0 1
((variable_name) @variable.builtin
  (#eq? @variable.builtin "$this"))

;; #C792EA #000000 0 0 0 1
(namespace_definition
  name: (namespace_name (name) @module))

;; #C792EA #000000 0 0 0 1
(namespace_name
  (name) @module)

;; #7AA2F7 #000000 0 0 0 1
(relative_name
  "namespace" @module.builtin)

;; #89DDFF #000000 0 0 0 1
(attribute_list) @attribute

;; #FF9D5C #000000 0 0 0 1
(conditional_expression
  "?" @keyword.conditional.ternary
  ":" @keyword.conditional.ternary)

;; #9CDCFE #000000 0 0 0 1
(declare_directive
  [
    "strict_types"
    "ticks"
    "encoding"
  ] @variable.parameter)

;; #A6E3A1 #000000 0 0 0 1
[
  (string)
  (encapsed_string)
  (heredoc_body)
  (nowdoc_body)
  (shell_command_expression)
] @string

;; #A6E3A1 #000000 0 0 0 1
(escape_sequence) @string.escape

;; #A6E3A1 #000000 0 0 0 1
[
  (heredoc_start)
  (heredoc_end)
] @label

;; #DDB6F2 #000000 0 0 0 1
(nowdoc
  "'" @label)

;; #F38BA8 #000000 0 0 0 1
(boolean) @boolean

;; #F38BA8 #000000 0 0 0 1
(null) @constant.builtin

;; #F38BA8 #000000 0 0 0 1
(integer) @number

;; #F38BA8 #000000 0 0 0 1
(float) @number.float

;; #99ADBF #000000 0 1 0 1
(comment) @comment @spell

;; #A6E3A1 #000000 0 0 0 1
(named_label_statement) @label

;; #7AA2F7 #000000 0 0 0 1
(property_hook
  (name) @label)

;; #7AA2F7 #000000 0 0 0 1
(visibility_modifier
  (operation) @label)

;; #89DDFF #000000 0 0 0 1
;; !html
(text) @injection.html

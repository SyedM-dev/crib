; ============================================================
; Identifiers
; ============================================================

;; #FFFFFF #000000 0 0 0 1
((identifier) @variable)

;; #FFB870 #000000 0 0 0 9
(function_declarator
  declarator: (identifier) @function)

;; #C4B5FF #000000 0 0 0 2
((identifier) @constant
  (#match? @constant "^[A-Z][A-Z0-9_]+$"))

;; #C4B5FF #000000 0 0 0 2
(preproc_def
  (preproc_arg) @constant
  (#match? @constant "^[A-Z][A-Z0-9_]+$"))

;; #F29CC3 #000000 0 0 0 2
((identifier) @constant.builtin
  (#match? @constant.builtin "^(stderr|stdin|stdout|__FILE__|__LINE__|__DATE__|__TIME__|__STDC__|__STDC_VERSION__|__STDC_HOSTED__|__cplusplus|__OBJC__|__ASSEMBLER__|__BASE_FILE__|__FILE_NAME__|__INCLUDE_LEVEL__|__TIMESTAMP__|__clang__|__clang_major__|__clang_minor__|__clang_patchlevel__|__clang_version__|__clang_literal_encoding__|__clang_wide_literal_encoding__|__FUNCTION__|__func__|__PRETTY_FUNCTION__|__VA_ARGS__|__VA_OPT__)$"))

;; #F29CC3 #000000 0 0 0 2
(preproc_def
  (preproc_arg) @constant.builtin
  (#match? @constant.builtin "^(stderr|stdin|stdout|__FILE__|__LINE__|__DATE__|__TIME__|__STDC__|__STDC_VERSION__|__STDC_HOSTED__|__cplusplus|__OBJC__|__ASSEMBLER__|__BASE_FILE__|__FILE_NAME__|__INCLUDE_LEVEL__|__TIMESTAMP__|__clang__|__clang_major__|__clang_minor__|__clang_patchlevel__|__clang_version__|__clang_literal_encoding__|__clang_wide_literal_encoding__|__FUNCTION__|__func__|__PRETTY_FUNCTION__|__VA_ARGS__|__VA_OPT__)$"))

;; #8AD5FF #000000 0 0 0 2
(preproc_def
  (preproc_arg) @variable)

;; #8AD5FF #000000 0 0 0 2
(statement_identifier) @label

;; #8AD5FF #000000 0 0 0 2
(declaration
  type: (type_identifier) @_type
  declarator: (identifier) @label
  (#match? @_type "^__label__$"))

;; #7CD5CF #000000 0 0 0 2
((identifier) @variable.member
  (#match? @variable.member "^m_.*$"))

; ============================================================
; Keywords
; ============================================================

;; #9AD4FF #000000 0 0 0 2
[
  "default"
  "goto"
  "asm"
  "__asm__"
] @keyword

;; #9AD4FF #000000 0 0 0 2
[
  "enum"
  "struct"
  "union"
  "typedef"
] @keyword.type

;; #F29CC3 #000000 0 0 0 2
[
  "sizeof"
  "offsetof"
] @keyword.operator

;; #F29CC3 #000000 0 0 0 2
(alignof_expression
  .
  _ @keyword.operator)

;; #FFB870 #000000 0 0 0 2
"return" @keyword.return

;; #9AD4FF #000000 0 0 0 2
[
  "while"
  "for"
  "do"
  "continue"
  "break"
] @keyword.repeat

;; #FFB870 #000000 0 0 0 2
[
  "if"
  "else"
  "case"
  "switch"
] @keyword.conditional

;; #9AD4FF #000000 0 0 0 2
(conditional_expression
  [
    "?"
    ":"
  ] @keyword.conditional.ternary)

;; #8AD5FF #000000 0 0 0 2
[
  "#if"
  "#ifdef"
  "#ifndef"
  "#else"
  "#elif"
  "#endif"
  "#elifdef"
  "#elifndef"
  (preproc_directive)
] @keyword.directive

;; #8AD5FF #000000 0 0 0 2
"#define" @keyword.directive.define

;; #8AD5FF #000000 0 0 0 2
"#include" @keyword.import

;; #9AD4FF #000000 0 0 0 2
[
  "try"
  "catch"
  "noexcept"
  "throw"
] @keyword.exception

;; #9AD4FF #000000 0 0 0 2
[
  "decltype"
  "explicit"
  "friend"
  "override"
  "using"
  "requires"
  "constexpr"
] @keyword

;; #9AD4FF #000000 0 0 0 2
[
  "class"
  "namespace"
  "template"
  "typename"
  "concept"
] @keyword.type

;; #9AD4FF #000000 0 0 0 2
[
  "co_await"
  "co_yield"
  "co_return"
] @keyword.coroutine

;; #F29CC3 #000000 0 0 0 2
[
  "public"
  "private"
  "protected"
  "final"
  "virtual"
] @keyword.modifier

;; #F29CC3 #000000 0 0 0 2
(storage_class_specifier) @keyword.modifier

;; #F29CC3 #000000 0 0 0 2
[
  (type_qualifier)
  (gnu_asm_qualifier)
  "__extension__"
] @keyword.modifier

;; #F29CC3 #000000 0 0 0 2
(linkage_specification
  "extern" @keyword.modifier)

;; #F29668 #000000 0 0 0 2
[
  "new"
  "delete"
  "xor"
  "bitand"
  "bitor"
  "compl"
  "not"
  "xor_eq"
  "and_eq"
  "or_eq"
  "not_eq"
  "and"
  "or"
] @keyword.operator

;; #F29668 #000000 0 1 0 2
"<=>" @operator

; ============================================================
; Types & modules
; ============================================================

;; #C4B5FF #000000 0 0 0 2
[
  (type_identifier)
  (type_descriptor)
] @type

;; #C4B5FF #000000 0 0 0 2
(type_definition
  declarator: (type_identifier) @type.definition)

;; #C4B5FF #000000 0 0 0 2
(primitive_type) @type.builtin

;; #C4B5FF #000000 0 0 0 2
(sized_type_specifier
  _ @type.builtin
  type: _?)

;; #9AD4FF #000000 0 0 0 2
(namespace_identifier) @module

;; #9AD4FF #000000 0 0 0 2
((namespace_identifier) @type
  (#match? @type "^[A-Z]"))

;; #9AD4FF #000000 0 0 0 2
(using_declaration
  .
  "using"
  .
  "namespace"
  .
  [
    (qualified_identifier)
    (identifier)
  ] @module)

; ============================================================
; Functions & calls
; ============================================================

;; #FFB870 #000000 0 0 0 1
(operator_name) @function

;; #FFB870 #000000 0 0 0 3
"operator" @function

;; #78C2FF #000000 0 0 0 2
(call_expression
  function: (identifier) @function.call)

;; #F29CC3 #000000 0 0 0 2
((call_expression
  function: (identifier) @function.builtin)
  (#match? @function.builtin "^__builtin_"))

;; #F29CC3 #000000 0 0 0 2
((call_expression
  function: (identifier) @function.builtin))

; ============================================================
; Constructors & methods
; ============================================================

;; #59C2FF #000000 0 0 0 2
((call_expression
  function: (identifier) @constructor)
  (#match? @constructor "^[A-Z]"))

;; #59C2FF #000000 0 0 0 2
((call_expression
  function: (qualified_identifier
    name: (identifier) @constructor))
  (#match? @constructor "^[A-Z]"))

;; #59C2FF #000000 0 0 0 2
((call_expression
  function: (field_expression
    field: (field_identifier) @constructor))
  (#match? @constructor "^[A-Z]"))

;; #59C2FF #000000 0 0 0 2
((field_initializer
  (field_identifier) @constructor
  (argument_list))
  (#match? @constructor "^[A-Z]"))

;; #59C2FF #000000 0 0 0 4
(destructor_name
  (identifier) @function.method)

; ============================================================
; Properties & members
; ============================================================

;; #F29CC3 #000000 0 0 0 2
((field_expression
  (field_identifier) @property) @_parent)

(field_designator) @property

((field_identifier) @property)

(field_initializer
  (field_identifier) @property)

;; #F29CC3 #000000 0 0 1 2
(field_declaration
  (field_identifier) @variable.member)

; ============================================================
; Parameters
; ============================================================

;; #7CD5CF #000000 0 0 0 2
(parameter_declaration
  declarator: (identifier) @variable.parameter)

;; #7CD5CF #000000 0 0 0 2
(parameter_declaration
  declarator: (array_declarator) @variable.parameter)

;; #7CD5CF #000000 0 0 0 2
(parameter_declaration
  declarator: (pointer_declarator) @variable.parameter)

;; #7CD5CF #000000 0 0 0 2
(preproc_params
  (identifier) @variable.parameter)

;; #7CD5CF #000000 0 0 0 2
(parameter_declaration
  declarator: (reference_declarator) @variable.parameter)

;; #7CD5CF #000000 0 0 0 2
(variadic_parameter_declaration
  declarator: (variadic_declarator
    (_) @variable.parameter))

;; #7CD5CF #000000 0 0 0 2
(optional_parameter_declaration
  declarator: (_) @variable.parameter)

; ============================================================
; Attributes & specifiers
; ============================================================

;; #7CD5CF #000000 0 0 0 2
[
  "__attribute__"
  "__declspec"
  "__based"
  "__cdecl"
  "__clrcall"
  "__stdcall"
  "__fastcall"
  "__thiscall"
  "__vectorcall"
  (ms_pointer_modifier)
  (attribute_declaration)
] @attribute

;; #7CD5CF #000000 0 0 0 2
(attribute_specifier
  (argument_list
    (identifier) @variable.builtin))

;; #7CD5CF #000000 0 0 0 2
(attribute_specifier
  (argument_list
    (call_expression
      function: (identifier) @variable.builtin)))

; ============================================================
; Operators & punctuation
; ============================================================

;; #F29668 #000000 0 1 0 1
[
  "="
  "-"
  "*"
  "/"
  "+"
  "%"
  "~"
  "|"
  "&"
  "^"
  "<<"
  ">>"
  "->"
  "<"
  "<="
  ">="
  ">"
  "=="
  "!="
  "!"
  "&&"
  "||"
  "-="
  "+="
  "*="
  "/="
  "%="
  "|="
  "&="
  "^="
  ">>="
  "<<="
  "--"
  "++"
] @operator

;; #F29668 #000000 0 1 0 1
(comma_expression
  "," @operator)

;; #B6BEC8 #000000 0 0 0 1
[
  ";"
  ":"
  ","
  "."
  "::"
] @punctuation.delimiter

;; #B6BEC8 #000000 0 0 0 1
"::" @punctuation.delimiter

;; #B6BEC8 #000000 0 0 0 1
"..." @punctuation.special

;; #B6BEC8 #000000 0 0 0 1
[
  "("
  ")"
  "["
  "]"
  "{"
  "}"
] @punctuation.bracket

;; #B6BEC8 #000000 0 0 0 1
(template_argument_list
  [
    "<"
    ">"
  ] @punctuation.bracket)

;; #B6BEC8 #000000 0 0 0 1
(template_parameter_list
  [
    "<"
    ">"
  ] @punctuation.bracket)

; ============================================================
; Literals
; ============================================================

;; #C2E8FF #000000 0 0 0 2
[
  (true)
  (false)
] @boolean

;; #C2E8FF #000000 0 0 0 2
(true) @boolean_true

;; #C2E8FF #000000 0 0 0 2
(false) @boolean_false

;; #A6E3A1 #000000 0 0 0 2
(string_literal) @string

;; #A6E3A1 #000000 0 0 0 2
(system_lib_string) @string

;; #A6E3A1 #000000 0 0 0 2
(raw_string_literal) @string

;; #A6E3A1 #000000 0 0 0 2
(escape_sequence) @string.escape

;; #B8E986 #000000 0 0 0 2
(number_literal) @number

;; #B8E986 #000000 0 0 0 2
(char_literal) @character

;; #F29CC3 #000000 0 0 0 2
(null) @constant.builtin

;; #F29CC3 #000000 0 0 0 2
(null
  "nullptr" @constant.builtin)

; ============================================================
; Macros & directives
; ============================================================

;; #F29CC3 #000000 0 0 0 2
(preproc_def
  name: (_) @constant.macro)

;; #F29CC3 #000000 0 0 0 2
(preproc_call
  directive: (preproc_directive) @_u
  argument: (_) @constant.macro
  (#match? @_u "^#undef$"))

;; #F29CC3 #000000 0 0 0 2
(preproc_ifdef
  name: (identifier) @constant.macro)

;; #F29CC3 #000000 0 0 0 2
(preproc_elifdef
  name: (identifier) @constant.macro)

;; #F29CC3 #000000 0 0 0 2
(preproc_defined
  (identifier) @constant.macro)

;; #F29CC3 #000000 0 0 0 2
(preproc_defined) @function.macro

; ============================================================
; Builtins & special identifiers
; ============================================================

;; #F28FAD #000000 0 0 0 2
(attribute_specifier
  (argument_list
    (identifier) @variable.builtin))

;; #F28FAD #000000 0 0 0 2
(attribute_specifier
  (argument_list
    (call_expression
      function: (identifier) @variable.builtin)))

;; #F28FAD #000000 0 0 0 2
(this) @variable.builtin

; ============================================================
; Exceptions & control helpers
; ============================================================

;; #FFB870 #000000 0 0 0 2
"static_assert" @function.builtin

; ============================================================
; Comments
; ============================================================

;; #99ADBF #000000 0 1 0 1
(comment) @comment @spell

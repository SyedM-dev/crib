;; #ffffff #000000 0 0 0 1
((identifier) @variable
  (#set! priority 95))

(preproc_def
  (preproc_arg) @variable)

;; #FF8F40 #000000 0 0 0 1
[
  "default"
  "goto"
  "asm"
  "__asm__"
] @keyword

[
  "enum"
  "struct"
  "union"
  "typedef"
] @keyword.type

[
  "sizeof"
  "offsetof"
] @keyword.operator

(alignof_expression
  .
  _ @keyword.operator)

;; #FF8F40 #000000 0 0 0 1
"return" @keyword.return

;; #FF8F40 #000000 0 0 0 1
[
  "while"
  "for"
  "do"
  "continue"
  "break"
] @keyword.repeat

;; #FF8F40 #000000 0 0 0 1
[
  "if"
  "else"
  "case"
  "switch"
] @keyword.conditional

;; #FF8F40 #000000 0 0 0 1
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

;; #FF8F40 #000000 0 0 0 1
"#define" @keyword.directive.define

;; #FF8F40 #000000 0 0 0 1
"#include" @keyword.import

;; #BFBDB6 #000000 0 0 0 1
[
  ";"
  ":"
  ","
  "."
  "::"
] @punctuation.delimiter

;; #BFBDB6 #000000 0 0 0 2
"..." @punctuation.special

;; #BFBDB6 #000000 0 0 0 1
[
  "("
  ")"
  "["
  "]"
  "{"
  "}"
] @punctuation.bracket

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

;; #F29668 #000000 0 0 0 1
(comma_expression
  "," @operator)

;; #D2A6FF #000000 0 0 0 1
[
  (true)
  (false)
] @boolean

;; #F29668 #000000 0 0 0 1
(conditional_expression
  [
    "?"
    ":"
  ] @keyword.conditional.ternary)

;; #aad84c #000000 0 0 0 1
(string_literal) @string

(system_lib_string) @string

;; #95E6CB #000000 0 0 0 2
(escape_sequence) @string.escape

;; #D2A6FF #000000 0 0 0 1
(null) @constant.builtin

;; #D2A6FF #000000 0 0 0 1
(number_literal) @number

;; #39BAE6 #000000 0 0 0 1
(char_literal) @character

;; #FFB454 #000000 0 0 0 1
(preproc_defined) @function.macro

;; #F07178 #000000 0 0 0 1
((field_expression
  (field_identifier) @property) @_parent)

(field_designator) @property

((field_identifier) @property)

;; #39BAE6 #000000 0 0 0 1
(statement_identifier) @label

(declaration
  type: (type_identifier) @_type
  declarator: (identifier) @label
  (#match? @_type "^__label__$"))

;; #59C2FF #000000 0 0 0 1
[
  (type_identifier)
  (type_descriptor)
] @type

;; #FF8F40 #000000 0 0 0 1
(storage_class_specifier) @keyword.modifier

[
  (type_qualifier)
  (gnu_asm_qualifier)
  "__extension__"
] @keyword.modifier

;; #FF8F40 #000000 0 0 0 1
(linkage_specification
  "extern" @keyword.modifier)

;; #59C2FF #000000 0 0 0 1
(type_definition
  declarator: (type_identifier) @type.definition)

;; #59C2FF #000000 0 0 0 1
(primitive_type) @type.builtin

(sized_type_specifier
  _ @type.builtin
  type: _?)

;; #D2A6FF #000000 0 0 0 1
((identifier) @constant
  (#match? @constant "^[A-Z][A-Z0-9_]+$"))

(preproc_def
  (preproc_arg) @constant
  (#match? @constant "^[A-Z][A-Z0-9_]+$"))

(enumerator
  name: (identifier) @constant)

(case_statement
  value: (identifier) @constant)

;; #D2A6FF #000000 0 0 0 1
((identifier) @constant.builtin
  (#match? @constant.builtin "^(stderr|stdin|stdout|__FILE__|__LINE__|__DATE__|__TIME__|__STDC__|__STDC_VERSION__|__STDC_HOSTED__|__cplusplus|__OBJC__|__ASSEMBLER__|__BASE_FILE__|__FILE_NAME__|__INCLUDE_LEVEL__|__TIMESTAMP__|__clang__|__clang_major__|__clang_minor__|__clang_patchlevel__|__clang_version__|__clang_literal_encoding__|__clang_wide_literal_encoding__|__FUNCTION__|__func__|__PRETTY_FUNCTION__|__VA_ARGS__|__VA_OPT__)$"))

(preproc_def
  (preproc_arg) @constant.builtin
  (#match? @constant.builtin "^(stderr|stdin|stdout|__FILE__|__LINE__|__DATE__|__TIME__|__STDC__|__STDC_VERSION__|__STDC_HOSTED__|__cplusplus|__OBJC__|__ASSEMBLER__|__BASE_FILE__|__FILE_NAME__|__INCLUDE_LEVEL__|__TIMESTAMP__|__clang__|__clang_major__|__clang_minor__|__clang_patchlevel__|__clang_version__|__clang_literal_encoding__|__clang_wide_literal_encoding__|__FUNCTION__|__func__|__PRETTY_FUNCTION__|__VA_ARGS__|__VA_OPT__)$"))

;; #D2A6FF #000000 0 0 0 1
(attribute_specifier
  (argument_list
    (identifier) @variable.builtin))

(attribute_specifier
  (argument_list
    (call_expression
      function: (identifier) @variable.builtin)))

;; #FFB454 #000000 0 0 0 1
((call_expression
  function: (identifier) @function.builtin)
  (#match? @function.builtin "^__builtin_"))

((call_expression
  function: (identifier) @function.builtin))

;; #D2A6FF #000000 0 0 0 1
(preproc_def
  name: (_) @constant.macro)

(preproc_call
  directive: (preproc_directive) @_u
  argument: (_) @constant.macro
  (#match? @_u "^#undef$"))

(preproc_ifdef
  name: (identifier) @constant.macro)

(preproc_elifdef
  name: (identifier) @constant.macro)

(preproc_defined
  (identifier) @constant.macro)

;; #FFB454 #000000 0 0 0 3
(call_expression
  function: (identifier) @function.call)

(call_expression
  function: (field_expression
    field: (field_identifier) @function.call))

;; #FFB454 #000000 0 0 0 3
(function_declarator
  declarator: (identifier) @function)

(function_declarator
  declarator: (parenthesized_declarator
    (pointer_declarator
      declarator: (field_identifier) @function)))

;; #FFB454 #000000 0 0 0 3
(preproc_function_def
  name: (identifier) @function.macro)

;; #99ADBF #000000 0 1 0 1
(comment) @comment @spell

;; #99ADBF #000000 0 1 0 1
((comment) @comment.documentation
  (#match? @comment.documentation "^/[*][*][^*].*[*]/$"))

;; #D2A6FF #000000 0 0 0 1
(parameter_declaration
  declarator: (identifier) @variable.parameter)

(parameter_declaration
  declarator: (array_declarator) @variable.parameter)

(parameter_declaration
  declarator: (pointer_declarator) @variable.parameter)

(preproc_params
  (identifier) @variable.parameter)

;; #FF8F40 #000000 0 0 0 1
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

;; #F07178 #000000 0 0 0 1
((identifier) @variable.member
  (#match? @variable.member "^m_.*$"))

(parameter_declaration
  declarator: (reference_declarator) @variable.parameter)

(variadic_parameter_declaration
  declarator: (variadic_declarator
    (_) @variable.parameter))

(optional_parameter_declaration
  declarator: (_) @variable.parameter)

;; #F07178 #000000 0 0 0 1
((field_expression
  (field_identifier) @function.method) @_parent)

(field_declaration
  (field_identifier) @variable.member)

(field_initializer
  (field_identifier) @property)

(function_declarator
  declarator: (field_identifier) @function.method)

;; #59C2FF #000000 0 0 0 3
(concept_definition
  name: (identifier) @type.definition)

(alias_declaration
  name: (type_identifier) @type.definition)

;; #59C2FF #000000 0 0 0 1
(auto) @type.builtin

;; #59C2FF #000000 0 0 0 1
(namespace_identifier) @module

;; #59C2FF #000000 0 0 0 1
((namespace_identifier) @type
  (#match? @type "^[A-Z]"))

;; #D2A6FF #000000 0 0 0 1
(case_statement
  value: (qualified_identifier
    (identifier) @constant))

;; #FF8F40 #000000 0 0 0 1
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

;; #FFB454 #000000 0 0 0 3
(destructor_name
  (identifier) @function.method)

;; #FFB454 #000000 0 0 0 3
(function_declarator
  (qualified_identifier
    (identifier) @function))

(function_declarator
  (qualified_identifier
    (qualified_identifier
      (identifier) @function)))

(function_declarator
  (qualified_identifier
    (qualified_identifier
      (qualified_identifier
        (identifier) @function))))

((qualified_identifier
  (qualified_identifier
    (qualified_identifier
      (qualified_identifier
        (identifier) @function)))) @_parent)

(function_declarator
  (template_function
    (identifier) @function))

(operator_name) @function

"operator" @function

;; #FFB454 #000000 0 0 0 3
"static_assert" @function.builtin

;; #FFB454 #000000 0 0 0 3
(call_expression
  (qualified_identifier
    (identifier) @function.call))

(call_expression
  (qualified_identifier
    (qualified_identifier
      (identifier) @function.call)))

(call_expression
  (qualified_identifier
    (qualified_identifier
      (qualified_identifier
        (identifier) @function.call))))

((qualified_identifier
  (qualified_identifier
    (qualified_identifier
      (qualified_identifier
        (identifier) @function.call)))) @_parent)

(call_expression
  (template_function
    (identifier) @function.call))

(call_expression
  (qualified_identifier
    (template_function
      (identifier) @function.call)))

(call_expression
  (qualified_identifier
    (qualified_identifier
      (template_function
        (identifier) @function.call))))

(call_expression
  (qualified_identifier
    (qualified_identifier
      (qualified_identifier
        (template_function
          (identifier) @function.call)))))

((qualified_identifier
  (qualified_identifier
    (qualified_identifier
      (qualified_identifier
        (template_function
          (identifier) @function.call))))) @_parent)

(function_declarator
  (template_method
    (field_identifier) @function.method))

;; #FFB454 #000000 0 0 0 3
(call_expression
  (field_expression
    (field_identifier) @function.method.call))

(call_expression
  (field_expression
    (template_method
      (field_identifier) @function.method.call)))

;; #FFB454 #000000 0 0 0 3
((function_declarator
  (qualified_identifier
    (identifier) @constructor))
  (#match? @constructor "^[A-Z]"))

((call_expression
  function: (identifier) @constructor)
  (#match? @constructor "^[A-Z]"))

((call_expression
  function: (qualified_identifier
    name: (identifier) @constructor))
  (#match? @constructor "^[A-Z]"))

((call_expression
  function: (field_expression
    field: (field_identifier) @constructor))
  (#match? @constructor "^[A-Z]"))

((field_initializer
  (field_identifier) @constructor
  (argument_list))
  (#match? @constructor "^[A-Z]"))

;; #F07178 #000000 0 0 0 1
(this) @variable.builtin

;; #D2A6FF #000000 0 0 0 1
(null
  "nullptr" @constant.builtin)

;; #D2A6FF #000000 0 0 0 2
(true) @boolean_true

;; #D2A6FF #000000 0 0 0 2
(false) @boolean_false

;; #aad84c #000000 0 0 0 1
(raw_string_literal) @string

;; #FF8F40 #000000 0 0 0 1
[
  "try"
  "catch"
  "noexcept"
  "throw"
] @keyword.exception

;; #FF8F40 #000000 0 0 0 1
[
  "decltype"
  "explicit"
  "friend"
  "override"
  "using"
  "requires"
  "constexpr"
] @keyword

;; #FF8F40 #000000 0 0 0 1
[
  "class"
  "namespace"
  "template"
  "typename"
  "concept"
] @keyword.type

;; #FF8F40 #000000 0 0 0 1
[
  "co_await"
  "co_yield"
  "co_return"
] @keyword.coroutine

;; #FF8F40 #000000 0 0 0 1
[
  "public"
  "private"
  "protected"
  "final"
  "virtual"
] @keyword.modifier

;; #FF8F40 #000000 0 0 0 1
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

;; #F29668 #000000 0 0 0 1
"<=>" @operator

;; #BFBDB6 #000000 0 0 0 1
"::" @punctuation.delimiter

;; #BFBDB6 #000000 0 0 0 1
(template_argument_list
  [
    "<"
    ">"
  ] @punctuation.bracket)

(template_parameter_list
  [
    "<"
    ">"
  ] @punctuation.bracket)

;; #FF8F40 #000000 0 0 0 1
(literal_suffix) @operator

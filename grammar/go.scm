;; #FFB454 #000000 0 0 0 0 3
(type_identifier) @type

;; #FFB454 #000000 0 0 0 0 3
(type_spec
  name: (type_identifier) @type.definition)

;; #FFFFFF #000000 0 0 0 0 1
(field_identifier) @property

;; #FFFFFF #000000 0 0 0 0 1
(identifier) @variable

;; #FFFFFF #000000 0 0 0 0 1
(package_identifier) @module

;; #FFFFFF #000000 0 0 0 0 1
(parameter_declaration
  (identifier) @variable.parameter)

;; #FFFFFF #000000 0 0 0 0 1
(variadic_parameter_declaration
  (identifier) @variable.parameter)

;; #F07178 #000000 0 0 0 0 1
(label_name) @label

;; #D2A6FF #000000 0 0 0 0 1
(const_spec
  name: (identifier) @constant)

;; #FFB454 #000000 0 0 0 0 3
(call_expression
  function: (identifier) @function.call)

;; #FFB454 #000000 0 0 0 0 3
(call_expression
  function: (selector_expression
    field: (field_identifier) @function.method.call))

;; #FFB454 #000000 0 0 0 0 3
(function_declaration
  name: (identifier) @function)

;; #FFB454 #000000 0 0 0 0 3
(method_declaration
  name: (field_identifier) @function.method)

;; #FFB454 #000000 0 0 0 0 3
(method_elem
  name: (field_identifier) @function.method)

;; #FFB454 #000000 0 0 0 0 3
((call_expression
  (identifier) @constructor)
  (#match? @constructor "^[nN]ew.+$"))

;; #FFB454 #000000 0 0 0 0 3
((call_expression
  (identifier) @constructor)
  (#match? @constructor "^[mM]ake.+$"))

;; #F29668 #000000 0 1 0 0 1
[
  "--"
  "-"
  "-="
  ":="
  "!"
  "!="
  "..."
  "*"
  "*="
  "/"
  "/="
  "&"
  "&&"
  "&="
  "&^"
  "&^="
  "%"
  "%="
  "^"
  "^="
  "+"
  "++"
  "+="
  "<-"
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
  "|"
  "|="
  "||"
  "~"
] @operator

;; #FF8F40 #000000 0 0 0 0 1
[
  "break"
  "const"
  "continue"
  "default"
  "defer"
  "goto"
  "range"
  "select"
  "var"
  "fallthrough"
] @keyword

;; #FF8F40 #000000 0 0 0 0 1
[
  "type"
  "struct"
  "interface"
] @keyword.type

;; #FF8F40 #000000 0 0 0 0 1
"func" @keyword.function

;; #FF8F40 #000000 0 0 0 0 1
"return" @keyword.return

;; #FF8F40 #000000 0 0 0 0 1
"go" @keyword.coroutine

;; #FF8F40 #000000 0 0 0 0 1
"for" @keyword.repeat

;; #FF8F40 #000000 0 0 0 0 1
[
  "import"
  "package"
] @keyword.import

;; #FF8F40 #000000 0 0 0 0 1
[
  "else"
  "case"
  "switch"
  "if"
] @keyword.conditional

;; #F07178 #000000 0 0 0 0 1
[
  "chan"
  "map"
] @type.builtin

;; #F07178 #000000 0 0 0 0 1
((type_identifier) @type.builtin
  (#match? @type.builtin
    "^(any|bool|byte|comparable|complex128|complex64|error|float32|float64|int|int16|int32|int64|int8|rune|string|uint|uint16|uint32|uint64|uint8|uintptr)$"))

;; #FFB454 #000000 0 0 0 0 3
((identifier) @function.builtin
  (#match? @function.builtin
    "^(append|cap|clear|close|complex|copy|delete|imag|len|make|max|min|new|panic|print|println|real|recover)$"))

;; #BFBDB6 #000000 0 0 0 0 1
"." @punctuation.delimiter

;; #BFBDB6 #000000 0 0 0 0 1
"," @punctuation.delimiter

;; #BFBDB6 #000000 0 0 0 0 1
":" @punctuation.delimiter

;; #BFBDB6 #000000 0 0 0 0 1
";" @punctuation.delimiter

;; #BFBDB6 #000000 0 0 0 0 1
"(" @punctuation.bracket

;; #BFBDB6 #000000 0 0 0 0 1
")" @punctuation.bracket

;; #BFBDB6 #000000 0 0 0 0 1
"{" @punctuation.bracket

;; #BFBDB6 #000000 0 0 0 0 1
"}" @punctuation.bracket

;; #BFBDB6 #000000 0 0 0 0 1
"[" @punctuation.bracket

;; #BFBDB6 #000000 0 0 0 0 1
"]" @punctuation.bracket

;; #AAD94C #000000 0 0 0 0 1
(interpreted_string_literal) @string

;; #AAD94C #000000 0 0 0 0 1
(raw_string_literal) @string

;; #AAD94C #000000 0 0 0 0 1
(rune_literal) @string

;; #AAD94C #000000 0 0 0 0 1
(escape_sequence) @string.escape

;; #D2A6FF #000000 0 0 0 0 2
(int_literal) @number

;; #D2A6FF #000000 0 0 0 0 2
(float_literal) @number.float

;; #D2A6FF #000000 0 0 0 0 2
(imaginary_literal) @number

;; #D2A6FF #000000 0 0 0 0 1
[
  (true)
  (false)
] @boolean

;; #D2A6FF #000000 0 0 0 0 1
[
  (nil)
  (iota)
] @constant.builtin

;; #FFFFFF #000000 0 0 0 0 1
(keyed_element
  .
  (literal_element
    (identifier) @variable.member))

;; #FFFFFF #000000 0 0 0 0 1
(field_declaration
  name: (field_identifier) @variable.member)

;; #99ADBF #000000 0 1 0 0 1
(comment) @comment @spell

;; #99ADBF #000000 0 1 0 0 1
(source_file
  .
  (comment)+ @comment.documentation)

;; #99ADBF #000000 0 1 0 0 1
(source_file
  (comment)+ @comment.documentation
  .
  (const_declaration))

;; #99ADBF #000000 0 1 0 0 1
(source_file
  (comment)+ @comment.documentation
  .
  (function_declaration))

;; #99ADBF #000000 0 1 0 0 1
(source_file
  (comment)+ @comment.documentation
  .
  (type_declaration))

;; #99ADBF #000000 0 1 0 0 1
(source_file
  (comment)+ @comment.documentation
  .
  (var_declaration))

;; #AAD94C #000000 0 0 0 0 1
(call_expression
  (selector_expression) @_function
  (#match? @_function
    "^(regexp\.Match|regexp\.MatchReader|regexp\.MatchString|regexp\.Compile|regexp\.CompilePOSIX|regexp\.MustCompile|regexp\.MustCompilePOSIX)$")
  (argument_list
    .
    [
;; !regex
      (raw_string_literal
        (raw_string_literal_content) @string.regexp)
      (interpreted_string_literal
        (interpreted_string_literal_content) @string.regexp)
    ]))

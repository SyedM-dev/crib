[
  (method)
  (singleton_method)
  (class)
  (module)
  (if)
  (else)
  (case)
  (when)
  (in)
  (do_block)
  (singleton_class)
  (heredoc_content)
  (lambda)
] @fold

;; #ffffff #000000 0 0 0 1
[
  (identifier)
  (global_variable)
] @variable

;; #fbb152 #000000 0 0 0 1
[
  "alias"
  "begin"
  "do"
  "end"
  "ensure"
  "module"
  "rescue"
  "then"
] @keyword

;; #fbb152 #000000 0 0 0 1
"class" @keyword.type


;; #fbb152 #000000 0 0 0 1
[
  "return"
  "yield"
] @keyword.return

;; #fbb152 #000000 0 0 0 1
[
  "and"
  "or"
  "in"
  "not"
] @keyword.operator

;; #fbb152 #000000 0 0 0 1
[
  "def"
  "undef"
] @keyword.function

(method
  "end" @keyword.function)


;; #fbb152 #000000 0 0 0 1
[
  "case"
  "else"
  "elsif"
  "if"
  "unless"
  "when"
  "then"
] @keyword.conditional

(if
  "end" @keyword.conditional)

;; #fbb152 #000000 0 0 0 1
[
  "for"
  "until"
  "while"
  "break"
  "redo"
  "retry"
  "next"
] @keyword.repeat

;; #ebda8c #000000 0 0 0 1
(constant) @constant

;; #fbb152 #000000 0 0 0 1
[
  "rescue"
  "ensure"
] @keyword.exception

;; #aad84c #000000 0 0 0 1
"defined?" @function

;; #aad84c #000000 0 0 0 3
(call
  receiver: (constant)? @type
  method: [
    (identifier)
    (constant)
;; #ff5689 #000000 0 0 0 2
  ] @function.call)

(alias
  (identifier) @function)

(setter
  (identifier) @function)

(method
  name: [
    (identifier) @function
    (constant) @type
  ])

(singleton_method
  name: [
    (identifier) @function
    (constant) @type
  ])

(class
  name: (constant) @type)

(module
  name: (constant) @type)

(superclass
  (constant) @type)

;; #ffffff #000000 0 0 0 1
[
  (class_variable)
  (instance_variable)
] @variable.member

((identifier) @keyword.modifier
  (#match? @keyword.modifier "^(private|protected|public)$" ))

;; #fbb152 #000000 0 0 0 1
(program
  (call
    (identifier) @keyword.import)
  (#match? @keyword.import "^(require|require_relative|load)$"))

;; #fbb152 #000000 0 0 0 2
((identifier) @constant.builtin
  (#match? @constant.builtin "^(__callee__|__dir__|__id__|__method__|__send__|__ENCODING__|__FILE__|__LINE__)$" ))

;; #aad84c #000000 0 0 0 1
((identifier) @function.builtin
  (#match? @function.builtin "^(attr_reader|attr_writer|attr_accessor|module_function)$" ))

((call
  !receiver
  method: (identifier) @function.builtin)
  (#match? @function.builtin "^(include|extend|prepend|refine|using)"))

((identifier) @keyword.exception
  (#match? @keyword.exception "^(raise|fail|catch|throw)" ))

;; #ffffff #000000 0 0 0 1
[
  (self)
  (super)
] @variable.builtin

;; #ffffff #000000 0 0 0 1
(method_parameters
  (identifier) @variable.parameter)

(lambda_parameters
  (identifier) @variable.parameter)

(block_parameters
  (identifier) @variable.parameter)

(splat_parameter
  (identifier) @variable.parameter)

(hash_splat_parameter
  (identifier) @variable.parameter)

(optional_parameter
  (identifier) @variable.parameter)

(destructured_parameter
  (identifier) @variable.parameter)

(block_parameter
  (identifier) @variable.parameter)

(keyword_parameter
  (identifier) @variable.parameter)

;; #aad84c #000000 0 0 0 1
[
  (string_content)
  (heredoc_content)
  "\""
  "`"
] @string

;; #fbb152 #000000 0 0 0 1
[
  (heredoc_beginning)
  (heredoc_end)
] @label

;; #bd9ae6 #000000 0 0 0 2
[
  (bare_symbol)
  (simple_symbol)
  (delimited_symbol)
  (hash_key_symbol)
] @string.special.symbol

;; #e6a24c #000000 0 0 0 2
(regex
  (string_content) @string.regexp)

;; #e6a24c #000000 0 0 0 2
(escape_sequence) @string.escape

;; #ebda8c #000000 0 0 0 2
(integer) @number

;; #ebda8c #000000 0 0 0 2
(float) @number.float

;; #51eeba #000000 0 0 0 1
(true) @boolean.true

;; #ee513a #000000 0 0 0 1
(false) @boolean.false

;; #ee8757 #000000 0 0 0 1
(nil) @constant.nil

;; #AAAAAA #000000 0 1 0 1
(comment) @comment

(program
  (comment)+ @comment.documentation
  (class))

(module
  (comment)+ @comment.documentation
  (body_statement
    (class)))

(class
  (comment)+ @comment.documentation
  (body_statement
    (method)))

(body_statement
  (comment)+ @comment.documentation
  (method))

;; #ffffff #000000 0 0 0 1
[
  "!"
  "="
  ">>"
  "<<"
  ">"
  "<"
  "**"
  "*"
  "/"
  "%"
  "+"
  "-"
  "&"
  "|"
  "^"
  "%="
  "+="
  "-="
  "*="
  "/="
  "=~"
  "!~"
  "?"
  ":"
] @operator

;; #ffffff #000000 0 1 0 1
[
  "=="
  "==="
  "<=>"
  "=>"
  "->"
  ">="
  "<="
  "||"
  "||="
  "&&="
  "&&"
  "!="
  ".."
  "..."
] @operator.ligature

;; #bd9ae6 #000000 0 0 0 1
[
  ","
  ";"
  "."
  "&."
  "::"
] @punctuation.delimiter

(pair
  ":" @punctuation.delimiter)

;; #bd9ae6 #000000 0 0 0 1
[
  "("
  ")"
  "["
  "]"
  "{"
  "}"
  "%w("
  "%i("
] @punctuation.bracket

(regex
  "/" @punctuation.bracket)

(block_parameters
  "|" @punctuation.bracket)

;; #e6a24c #000000 0 0 0 2
(interpolation
  "#{" @punctuation.special
  "}" @punctuation.special)

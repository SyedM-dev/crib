;; #FFFFFF #000000 0 0 0 0 1
[
  (identifier)
  (global_variable)
] @variable

;; #FF8F40 #000000 0 0 0 0 1
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

;; #FF8F40 #000000 0 0 0 0 1
"class" @keyword.type


;; #FF8F40 #000000 0 0 0 0 1
[
  "return"
  "yield"
] @keyword.return

;; #F29668 #000000 0 0 0 0 1
[
  "and"
  "or"
  "in"
  "not"
] @keyword.operator

;; #FF8F40 #000000 0 0 0 0 1
[
  "def"
  "undef"
] @keyword.function

(method
  "end" @keyword.function)


;; #FF8F40 #000000 0 0 0 0 1
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

;; #FF8F40 #000000 0 0 0 0 1
[
  "for"
  "until"
  "while"
  "break"
  "redo"
  "retry"
  "next"
] @keyword.repeat

;; #D2A6FF #000000 0 0 0 0 1
(constant) @constant

;; #FF8F40 #000000 0 0 0 0 1
[
  "rescue"
  "ensure"
] @keyword.exception

;; #FFB454 #000000 0 0 0 0 3
"defined?" @function

;; #FFB454 #000000 0 0 0 0 3
(call
  receiver: (constant)? @type
  method: [
    (identifier)
    (constant)
;; #FFB454 #000000 0 0 0 0 2
  ] @function.call)

;; #FFB454 #000000 0 0 0 0 2
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

;; #59C2FF #000000 0 0 0 0 2
(class
  name: (constant) @type)

(module
  name: (constant) @type)

(superclass
  (constant) @type)

;; #F07178 #000000 0 0 0 0 2
[
  (class_variable)
  (instance_variable)
] @variable.member

;; #FF8F40 #000000 0 0 0 0 2
((identifier) @keyword.modifier
  (#match? @keyword.modifier "^(private|protected|public)$" ))

;; #FF8F40 #000000 0 0 0 0 3
(program
  (call
    (identifier) @keyword.import)
  (#match? @keyword.import "^(require|require_relative|load)$"))

;; #D2A6FF #000000 0 0 0 0 4
((identifier) @constant.builtin
  (#match? @constant.builtin "^(__callee__|__dir__|__id__|__method__|__send__|__ENCODING__|__FILE__|__LINE__)$" ))

;; #FFB454 #000000 0 0 0 0 3
((identifier) @function.builtin
  (#match? @function.builtin "^(attr_reader|attr_writer|attr_accessor|module_function)$" ))

((call
  !receiver
  method: (identifier) @function.builtin)
  (#match? @function.builtin "^(include|extend|prepend|refine|using)$"))

;; #FF8F40 #000000 0 0 0 0 3
((identifier) @keyword.exception
  (#match? @keyword.exception "^(raise|fail|catch|throw)$" ))

;; #F07178 #000000 0 0 0 0 1
[
  (self)
  (super)
] @variable.builtin

;; #D2A6FF #000000 0 0 0 0 1
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

;; #AAD94C #000000 0 0 0 0 1
[
  (string_content)
  (heredoc_content)
  "\""
  "`"
] @string

;; #E6C08A #000000 0 0 0 0 1
[
  (heredoc_beginning)
  (heredoc_end)
] @label

;; #39BAE6 #000000 0 0 0 0 2
[
  (bare_symbol)
  (simple_symbol)
  (delimited_symbol)
  (hash_key_symbol)
] @string.special.symbol

;; #95E6CB #000000 0 0 0 0 2
(escape_sequence) @string.escape

;; #D2A6FF #000000 0 0 0 0 2
(integer) @number

;; #D2A6FF #000000 0 0 0 0 2
(float) @number.float

;; #D2A6FF #000000 0 0 0 0 1
(true) @boolean.true

;; #D2A6FF #000000 0 0 0 0 1
(false) @boolean.false

;; #D2A6FF #000000 0 0 0 0 1
(nil) @constant.nil

;; #99ADBF #000000 0 1 0 0 1
(comment) @comment

;; #AAD94C #000000 0 0 0 0 3
((program
  .
  (comment) @shebang @nospell)
  (#match? @shebang "^#!/"))

;; #F29668 #000000 0 0 0 0 1
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

;; #F29668 #000000 0 1 0 0 1
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

;; #BFBDB6 #000000 0 0 0 0 1
[
  ","
  ";"
  "."
  "&."
  "::"
] @punctuation.delimiter

(pair
  ":" @punctuation.delimiter)

;; #BFBDB6 #000000 0 0 0 0 3
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

;; #7dcfff #000000 0 0 0 0 2
(interpolation
  "#{" @punctuation.special
  "}" @punctuation.special)

; Injections

;; !regex
(regex
  (string_content) @string.regexp)

(heredoc_body
;; !bash
  (heredoc_content) @bash_injection
  ((heredoc_end) @lang
    (#match? @lang "BASH")))

(heredoc_body
;; !c
  (heredoc_content) @c_injection
  ((heredoc_end) @lang
    (#match? @lang "C$")))

(heredoc_body
;; !cpp
  (heredoc_content) @cpp_injection
  ((heredoc_end) @lang
    (#match? @lang "CPP")))

(heredoc_body
;; !css
  (heredoc_content) @css_injection
  ((heredoc_end) @lang
    (#match? @lang "CSS")))

(heredoc_body
;; !fish
  (heredoc_content) @fish_injection
  ((heredoc_end) @lang
    (#match? @lang "FISH")))

(heredoc_body
;; !go
  (heredoc_content) @go_injection
  ((heredoc_end) @lang
    (#match? @lang "GO")))

(heredoc_body
;; !haskell
  (heredoc_content) @haskell_injection
  ((heredoc_end) @lang
    (#match? @lang "HASKELL")))

(heredoc_body
;; !html
  (heredoc_content) @html_injection
  ((heredoc_end) @lang
    (#match? @lang "HTML")))

(heredoc_body
;; !javascript
  (heredoc_content) @javascript_injection
  ((heredoc_end) @lang
    (#match? @lang "JAVASCRIPT")))

(heredoc_body
;; !json
  (heredoc_content) @json_injection
  ((heredoc_end) @lang
    (#match? @lang "JSON")))

(heredoc_body
;; !lua
  (heredoc_content) @lua_injection
  ((heredoc_end) @lang
    (#match? @lang "LUA")))

(heredoc_body
;; !make
  (heredoc_content) @make_injection
  ((heredoc_end) @lang
    (#match? @lang "MAKE")))

(heredoc_body
;; !python
  (heredoc_content) @python_injection
  ((heredoc_end) @lang
    (#match? @lang "PYTHON")))

(heredoc_body
;; !ruby
  (heredoc_content) @ruby_injection
  ((heredoc_end) @lang
    (#match? @lang "RUBY")))

(heredoc_body
;; !rust
  (heredoc_content) @rust_injection
  ((heredoc_end) @lang
    (#match? @lang "RUST")))

(heredoc_body
;; !diff
  (heredoc_content) @diff_injection
  ((heredoc_end) @lang
    (#match? @lang "DIFF")))

(heredoc_body
;; !embedded_template
  (heredoc_content) @embedded_template_injection
  ((heredoc_end) @lang
    (#match? @lang "ERB")))

(heredoc_body
;; !gdscript
  (heredoc_content) @gdscript_injection
  ((heredoc_end) @lang
    (#match? @lang "GDSCRIPT")))

(heredoc_body
;; !gitattributes
  (heredoc_content) @gitattributes_injection
  ((heredoc_end) @lang
    (#match? @lang "GITATTRIBUTES")))

(heredoc_body
;; !gitignore
  (heredoc_content) @gitignore_injection
  ((heredoc_end) @lang
    (#match? @lang "GITIGNORE")))

(heredoc_body
;; !gomod
  (heredoc_content) @gomod_injection
  ((heredoc_end) @lang
    (#match? @lang "GOMOD")))

(heredoc_body
;; !ini
  (heredoc_content) @ini_injection
  ((heredoc_end) @lang
    (#match? @lang "INI")))

(heredoc_body
;; !markdown
  (heredoc_content) @markdown_injection
  ((heredoc_end) @lang
    (#match? @lang "MARKDOWN")))

(heredoc_body
;; !nginx
  (heredoc_content) @nginx_injection
  ((heredoc_end) @lang
    (#match? @lang "NGINX")))

(heredoc_body
;; !php
  (heredoc_content) @php_injection
  ((heredoc_end) @lang
    (#match? @lang "PHP")))

(heredoc_body
;; !query
  (heredoc_content) @query_injection
  ((heredoc_end) @lang
    (#match? @lang "QUERY")))

(heredoc_body
;; !regex
  (heredoc_content) @regex_injection
  ((heredoc_end) @lang
    (#match? @lang "REGEX")))

(heredoc_body
;; !sql
  (heredoc_content) @sql_injection
  ((heredoc_end) @lang
    (#match? @lang "SQL")))

(heredoc_body
;; !toml
  (heredoc_content) @toml_injection
  ((heredoc_end) @lang
    (#match? @lang "TOML")))

(heredoc_body
;; !yaml
  (heredoc_content) @yaml_injection
  ((heredoc_end) @lang
    (#match? @lang "YAML")))

(heredoc_body
;; !cabal
  (heredoc_content) @cabal_injection
  ((heredoc_end) @lang
    (#match? @lang "CABAL")))

(heredoc_body
;; !man
  (heredoc_content) @man_injection
  ((heredoc_end) @lang
    (#match? @lang "MAN")))

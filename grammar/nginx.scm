;; #99ADBF #000000 0 1 0 1
(comment) @comment

;; #7dcfff #000000 0 0 0 2
(number) @number
(metric) @number

;; !regex
(regex) @regex

;; #FFFFFF #000000 0 0 0 1
(variable) @variable

;; #F29668 #000000 0 0 0 1
(modifier) @operator

;; #D2A6FF #000000 0 0 0 1
(simple_directive
  name: (directive) @function)

;; #D2A6FF #000000 0 0 0 1
(block_directive
  name: (directive) @function)

;; #D2A6FF #000000 0 0 0 1
(lua_block_directive
  "access_by_lua_block" @function)

;; #F07178 #000000 0 0 0 1
((generic) @constant.builtin
  (#match? @constant.builtin "^(off|on)$"))

;; #AAD94C #000000 0 0 0 2
(generic) @string
(string) @string

;; #FFFFFF #000000 0 0 0 1
(scheme) @string
(ipv4) @number

;; #888888 #000000 0 1 0 3
[
  ";"
] @delimiter

;; #888888 #000000 0 0 0 3
[
  "{"
  "}"
  "("
  ")"
  "["
  "]"
] @punctuation.bracket

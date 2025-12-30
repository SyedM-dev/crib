;; #D2A6FF #000000 0 0 0 0 1
[
  "require"
  "replace"
  "toolchain"
  "exclude"
  "retract"
] @keyword

;; #F07178 #000000 0 0 0 0 1
[
  "go"
  "module"
] @keyword.directive

;; #F29668 #000000 0 1 0 0 1
"=>" @operator

;; #99ADBF #000000 0 1 0 0 1
(comment) @comment @spell

;; #7dcfff #000000 0 0 0 0 1
(module_path) @string.special.url

;; #D2A6FF #000000 0 0 0 0 1
(tool_directive) @keyword.directive

(tool) @string.special.url

;; #F29668 #000000 0 0 0 0 2
[
  (version)
  (go_version)
  (toolchain_name)
] @string.special

;; #888888 #000000 0 0 0 0 3
[
  "("
  ")"
  "["
  "]"
] @punctuation.bracket

;; #888888 #000000 0 1 0 0 3
"," @punctuation.delimiter

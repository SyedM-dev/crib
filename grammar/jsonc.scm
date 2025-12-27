;; #D2A6FF #000000 0 0 0 2
(pair
  key: (_) @string.special.key)

;; #AAD94C #000000 0 0 0 1
(string) @string

;; #7dcfff #000000 0 0 0 2
(number) @number

;; #F07178 #000000 0 0 0 1
[
  (null)
  (true)
  (false)
] @constant.builtin

;; #7dcfff #000000 0 0 0 2
(escape_sequence) @escape

;; #99ADBF #000000 0 1 0 1
(comment) @comment

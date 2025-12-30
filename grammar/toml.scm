;; #F0F8FF #000000 0 0 0 0 2
(bare_key) @type

;; #FFFFFF #000000 0 0 0 0 1
(quoted_key) @string.quoted

;; #D2A6FF #000000 0 0 0 0 0
(pair
  (bare_key)) @property

;; #D2A6FF #000000 0 0 0 0 0
(pair
  (dotted_key
    (bare_key) @property))

;; #F29668 #000000 0 0 0 0 1
(boolean) @boolean

;; #99ADBF #000000 0 1 0 0 1
(comment) @comment

;; #AAD94C #000000 0 0 0 0 1
(string) @string

;; #7dcfff #000000 0 0 0 0 2
[
  (integer)
  (float)
] @number

;; #FFFFFF #000000 0 0 0 0 1
[
  (offset_date_time)
  (local_date_time)
  (local_date)
  (local_time)
] @string.special

;; #888888 #000000 0 1 0 0 3
[
  "."
  ","
] @punctuation.delimiter

;; #F29668 #000000 0 0 0 0 1
"=" @operator

;; #888888 #000000 0 0 0 0 3
[
  "["
  "]"
  "[["
  "]]"
  "{"
  "}"
] @punctuation.bracket

;; #F29668 #000000 0 0 0 1
(boolean_scalar) @boolean

;; #F07178 #000000 0 0 0 1
(null_scalar) @constant.builtin

;; #AAD94C #000000 0 0 0 0
[
  (double_quote_scalar)
  (single_quote_scalar)
] @string

;; #FFFFFF #000000 0 0 0 0
[
  (block_scalar)
  (string_scalar)
] @string.abs

;; #7dcfff #000000 0 0 0 2
[
  (integer_scalar)
  (float_scalar)
] @number

;; #99ADBF #000000 0 1 0 1
(comment) @comment

;; #D2A6FF #000000 0 0 0 1
[
  (anchor_name)
  (alias_name)
] @label

;; #7dcfff #000000 0 0 0 2
(tag) @type

;; #F07178 #000000 0 0 0 1
[
  (yaml_directive)
  (tag_directive)
  (reserved_directive)
] @attribute

;; #D2A6FF #000000 0 0 0 1
(block_mapping_pair
  key: (flow_node
    [
      (double_quote_scalar)
      (single_quote_scalar)
    ] @property))

;; #D2A6FF #000000 0 0 0 1
(block_mapping_pair
  key: (flow_node
    (plain_scalar
      (string_scalar) @property)))

;; #D2A6FF #000000 0 0 0 1
(flow_mapping
  (_
    key: (flow_node
      [
        (double_quote_scalar)
        (single_quote_scalar)
      ] @property)))

;; #D2A6FF #000000 0 0 0 1
(flow_mapping
  (_
    key: (flow_node
      (plain_scalar
        (string_scalar) @property))))

;; #F38BA8 #000000 0 1 0 3
[
  ","
  "-"
  ":"
  ">"
  "?"
  "|"
] @punctuation.delimiter

;; #888888 #000000 0 0 0 3
[
  "["
  "]"
  "{"
  "}"
] @punctuation.bracket

;; #AAD94C #000000 0 1 0 3
[
  "*"
  "&"
  "---"
  "..."
] @punctuation.special

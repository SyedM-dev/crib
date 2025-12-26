;; #7dcfff #000000 0 0 0 2
(section_name
  (text) @type)

;; #99ADBF #000000 0 1 0 1
(comment) @comment @spell

;; #888888 #000000 0 0 0 3
[
  "["
  "]"
] @punctuation.bracket

;; #F29668 #000000 0 1 0 1
"=" @operator

;; #F0F8FF #000000 0 0 0 2
(setting
  (setting_name) @property)

;; #F29668 #000000 0 0 0 2
((setting_value) @boolean
  (#match? @boolean "^\\s*(true|false|True|False|yes|no|Yes|No|on|off|On|Off|)\\s*$"))

;; #FF8F40 #000000 0 0 0 2
((setting_value) @number
   (#match? @number "^\\s*[-+0-9]+\\s*$"))

;; #A6E3A1 #000000 0 0 0 2
((setting_value) @float
  (#match? @float "^\\s*[-+0-9\\.]+\\s*$"))

;; #AAD94C #000000 0 0 0 1
(setting_value) @string

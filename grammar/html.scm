;; #99ADBF #000000 0 1 0 5
(comment) @comment @spell

;; #9ADE7A #000000 0 0 0 1
(attribute_name) @tag.attribute

;; #FF8F40 #000000 0 0 0 0
((attribute
  (quoted_attribute_value) @string))

(attribute_value) @string

[
 "'"
 "\""
] @string

;; #82AAFF #000000 1 0 0 3
((element
  (start_tag
    (tag_name) @_tag)
  (text) @markup.heading)
  (#match? @_tag "^title$"))

;; #82AAFF #000000 1 0 1 3
((element
  (start_tag
    (tag_name) @_tag)
  (text) @markup.heading.1)
  (#match? @_tag "^h[1-6]$"))

;; #FFD700 #000000 1 0 0 2
((element
  (start_tag
    (tag_name) @_tag)
  (text) @markup.strong)
  (#match? @_tag "^(strong|b)$"))

;; #FF8F40 #000000 0 1 0 2
((element
  (start_tag
    (tag_name) @_tag)
  (text) @markup.italic)
  (#match? @_tag "^(em|i)$"))

;; #FF6347 #000000 0 0 1 2
((element
  (start_tag
    (tag_name) @_tag)
  (text) @markup.strikethrough)
  (#match? @_tag "^(s|del)$"))

;; #82AAFF #000000 0 0 1 2
((element
  (start_tag
    (tag_name) @_tag)
  (text) @markup.underline)
  (#match? @_tag "^u$"))

;; #9ADE7A #000000 0 0 0 2
((element
  (start_tag
    (tag_name) @_tag)
  (text) @markup.raw)
  (#match? @_tag "^(code|kbd)$"))

;; #7dcfff #000000 0 0 1 2
((element
  (start_tag
    (tag_name) @_tag)
  (text) @markup.link.label)
  (#match? @_tag "^a$"))

((attribute
  (attribute_name) @_attr
  (quoted_attribute_value
;; #7dcfff #000000 0 0 1 5
    (attribute_value) @string.special.url))
  (#match? @_attr "^(href|src)$"))

;; Punctuation
;; #BFBDB6 #000000 0 0 0 1
[
  "<"
  ">"
  "</"
  "/>"
] @tag.delimiter

;; #FFFFFF #000000 0 1 0 1
"=" @operator

;; #7dcfff #000000 0 0 0 1
(tag_name) @tag

;; #FF8F40 #000000 0 0 0 1
(erroneous_end_tag_name) @tag.error

;; #FFD700 #000000 0 0 0 1
(doctype) @constant

;; #9ADE7A #000000 0 0 0 1
(attribute_name) @attribute

; Injections

((style_element
  (start_tag) @_start_tag
  ;; !css
  (raw_text) @injection.css))

((attribute
  (attribute_name) @_attr
  (quoted_attribute_value
    (attribute_value) @injection.css))
  (#match? @_attr "^style$"))

((script_element
  (start_tag) @_start
  ;; !javascript
  (raw_text) @injection.javascript)
  (#match? @_start "^<script\\b(?![^>]*\\btype\\s*=\\s*\\\"(?!module|text/javascript)[^\\\"]*\\\")[^>]*>$"))

((attribute
  (attribute_name) @_name
  (quoted_attribute_value
    (attribute_value) @injection.javascript))
  (#match? @_name "^on[a-z]+$"))

((attribute
  (quoted_attribute_value
    (attribute_value) @injection.javascript))
  (#match? @injection.javascript "\\$\\{"))

((script_element
  (start_tag) @_start
  ;; !json
  (raw_text) @injection.json)
  (#match? @_start "^<script\\b(?![^>]*\\btype\\s*=\\s*\\\"(?!importmap)[^\\\"]*\\\")[^>]*>$"))

((attribute
  (attribute_name) @_name
  (quoted_attribute_value
  ;; !regex
    (attribute_value) @injection.regex))
  (#match? @_name "^pattern$"))

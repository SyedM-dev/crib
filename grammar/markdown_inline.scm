;; #AAD94C #000000 0 0 0 0
(code_span) @markup.raw

;; #FF8F40 #000000 0 1 0 1
(emphasis) @markup.italic

;; #FFD700 #000000 1 0 0 1
(strong_emphasis) @markup.strong

;; #FF6347 #000000 0 0 1 1
(strikethrough) @markup.strikethrough

;; #7dcfff #000000 0 0 0 1
[
  (backslash_escape)
  (hard_line_break)
] @string.escape

;; #7dcfff #000000 0 0 1 1
(inline_link
  [
    "["
    "]"
    "("
    (link_destination)
    ")"
  ] @markup.link)

;; #7dcfff #000000 0 0 1 1
[
  (link_label)
  (link_text)
  (link_title)
  (image_description)
] @markup.link.label

;; #7dcfff #000000 0 0 1 1
((inline_link
  (link_destination) @_url) @_label)

;; #7dcfff #000000 0 0 1 1
((image
  (link_destination) @_url) @_label)

;; #7dcfff #000000 0 0 1 1
(image
  [
    "!"
    "["
    "]"
    "("
    (link_destination)
    ")"
  ] @markup.link)

;; #7dcfff #000000 0 0 1 1
(full_reference_link
  [
    "["
    "]"
    (link_label)
  ] @markup.link)

;; #7dcfff #000000 0 0 1 1
(collapsed_reference_link
  [
    "["
    "]"
  ] @markup.link)

;; #7dcfff #000000 0 0 1 1
(shortcut_link
  [
    "["
    "]"
  ] @markup.link)

;; #7dcfff #000000 0 0 1 1
[
  (link_destination)
  (uri_autolink)
  (email_autolink)
] @markup.link.url @nospell

;; #7dcfff #000000 0 0 1 1
(uri_autolink) @_url

;; #FF8F40 #000000 0 0 0 0
;; !html
(html_tag) @injection.html

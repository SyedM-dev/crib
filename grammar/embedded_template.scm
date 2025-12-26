;; #99ADBF #000000 0 1 0 4
(comment_directive) @comment

;; #F29668 #000000 0 0 0 6
[
  "<%#"
  "<%"
  "<%="
  "<%_"
  "<%-"
  "%>"
  "-%>"
  "_%>"
] @keyword

;; !html
(content) @injection.html

;; !ruby
(code) @injection.ruby

;; #82AAFF #000000 1 0 1 0 2
(title) @markup.heading.1

;; #ccefc9 #000000 0 0 0 0 0
(section_title) @markup.heading.2

;; #FF8F40 #000000 1 0 0 0 2
(subsection_title) @markup.heading.3

;; #AAD94C #000000 0 0 0 0 3
(option) @variable.parameter

;; #FFD700 #000000 1 0 0 0 3
(reference) @markup.link.label

;; #C792EA #000000 0 0 0 0 3
(footer) @markup.heading

(section_heading
  (section_title) @_title
;; #FFD700 #000000 1 0 0 0 1
  (block) @injection.content
  (#match? @_title "SYNOPSIS"))

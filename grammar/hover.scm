;; #82AAFF #000000 1 0 1 4
(setext_heading
  (paragraph) @markup.heading.1
  (setext_h1_underline) @markup.heading.1)

;; #82AAFF #000000 1 0 1 4
(setext_heading
  (paragraph) @markup.heading.2
  (setext_h2_underline) @markup.heading.2)

(atx_heading
  (atx_h1_marker)) @markup.heading.1

(atx_heading
  (atx_h2_marker)) @markup.heading.2

;; #82AAFF #000000 1 0 0 4
(atx_heading
  (atx_h3_marker)) @markup.heading.3

;; #82AAFF #000000 1 0 0 4
(atx_heading
  (atx_h4_marker)) @markup.heading.4

;; #82AAFF #000000 1 0 0 4
(atx_heading
  (atx_h5_marker)) @markup.heading.5

;; #82AAFF #000000 1 0 0 4
(atx_heading
  (atx_h6_marker)) @markup.heading.6

;; #82AAFF #000000 0 0 0 4
(info_string) @label

;; #FF6347 #000000 0 0 0 4
(pipe_table_header
  (pipe_table_cell) @markup.heading)

;; #FF8F40 #000000 0 0 0 4
(pipe_table_header
  "|" @punctuation.special)

(pipe_table_row
  "|" @punctuation.special)

(pipe_table_delimiter_row
  "|" @punctuation.special)

(pipe_table_delimiter_cell) @punctuation.special

;; #AAD94C #000000 0 0 0 2
(indented_code_block) @markup.raw.block

(fenced_code_block) @markup.raw.block

(fenced_code_block
  (fenced_code_block_delimiter) @markup.raw.block)

(fenced_code_block
  (info_string
    (language) @label))

;; #7dcfff #000000 0 0 1 6
(link_destination) @markup.link.url

;; #7dcfff #000000 0 0 1 6
[
  (link_title)
  (link_label)
] @markup.link.label

;; #FF8F40 #000000 0 0 0 4
((link_label)
  .
  ":" @punctuation.delimiter)

;; #9ADE7A #000000 0 0 0 4
[
  (list_marker_plus)
  (list_marker_minus)
  (list_marker_star)
  (list_marker_dot)
  (list_marker_parenthesis)
] @markup.list

(thematic_break) @punctuation.special

;; #FF8F40 #000000 0 0 0 4
(task_list_marker_unchecked) @markup.list.unchecked

;; #AAD94C #000000 0 0 0 4
(task_list_marker_checked) @markup.list.checked

[
  (plus_metadata)
  (minus_metadata)
] @keyword.directive

[
  (block_continuation)
  (block_quote_marker)
] @punctuation.special

;; #AAD94C #000000 0 0 0 6
(backslash_escape) @string.escape

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^ruby$"))
;; !ruby
  (code_fence_content) @injection.ruby)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^bash$"))
;; !bash
  (code_fence_content) @injection.bash)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^cpp$"))
;; !cpp
  (code_fence_content) @injection.cpp)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^h$"))
;; !h
  (code_fence_content) @injection.h)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^c$"))
;; !c
  (code_fence_content) @injection.h)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^css$"))
;; !css
  (code_fence_content) @injection.css)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^fish$"))
;; !fish
  (code_fence_content) @injection.fish)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^go$"))
;; !go
  (code_fence_content) @injection.go)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^haskell$"))
;; !haskell
  (code_fence_content) @injection.haskell)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^html$"))
;; !html
  (code_fence_content) @injection.html)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^javascript$"))
;; !javascript
  (code_fence_content) @injection.javascript)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^json$"))
;; !json
  (code_fence_content) @injection.json)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^lua$"))
; - lua format in hover boxes is typed making it unparsable as normal lua
; - TODO: add a lua grammar with typing or remove this injection
  (code_fence_content) @injection.lua)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^regex$"))
;; !regex
  (code_fence_content) @injection.regex)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^query$"))
;; !query
  (code_fence_content) @injection.query)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^markdown$"))
;; !markdown
  (code_fence_content) @injection.markdown)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^markdown_inline$"))
;; !markdown_inline
  (code_fence_content) @injection.markdown_inline)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^erb$"))
;; !embedded_template
  (code_fence_content) @injection.embedded_template)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^python$"))
;; !python
  (code_fence_content) @injection.python)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^php$"))
;; !php
  (code_fence_content) @injection.php)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^rust$"))
;; !rust
  (code_fence_content) @injection.rust)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^sql$"))
;; !sql
  (code_fence_content) @injection.sql)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^gitattributes$"))
;; !gitattributes
  (code_fence_content) @injection.gitattributes)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^gitignore$"))
;; !gitignore
  (code_fence_content) @injection.gitignore)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^gdscript$"))
;; !gdscript
  (code_fence_content) @injection.gdscript)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^make$"))
;; !make
  (code_fence_content) @injection.make)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^diff$"))
;; !diff
  (code_fence_content) @injection.diff)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^ini$"))
;; !ini
  (code_fence_content) @injection.ini)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^nginx$"))
;; !nginx
  (code_fence_content) @injection.nginx)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^toml$"))
;; !toml
  (code_fence_content) @injection.toml)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^yaml$"))
;; !yaml
  (code_fence_content) @injection.yaml)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^gomod$"))
;; !gomod
  (code_fence_content) @injection.gomod)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^man$"))
;; !man
  (code_fence_content) @injection.man)

(fenced_code_block
  (info_string
    (language) @injection.language (#match? @injection.language "^cabal$"))
;; !cabal
  (code_fence_content) @injection.cabal)

;; !html
(html_block) @injection.html

;; !yaml
(minus_metadata) @injection.yaml

;; !toml
(plus_metadata) @injection.toml

;; !markdown_inline
(paragraph) @inline

(pipe_table_row
  (pipe_table_cell) @inline)

(block_quote ((paragraph) @inline))

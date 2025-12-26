;; #F29668 #000000 0 1 0 1
[
  "&&"
  "||"
  "|"
  "&|"
  "2>|"
  "&"
  ".."
  "!"
  (direction)
  (stream_redirect)
] @operator

(command
  name: (word) @function.builtin
  (#match? @function.builtin "^test$")
;; #FFFFFF #000000 0 0 0 3
  argument: (word) @word
  (#match? @word "^(!?=|-[a-zA-Z]+)$"))

(command
  name: (word) @punctuation.bracket
  (#match? @punctuation.bracket "^\\[$")
  argument: (word) @word
  (#match? @word "^(!?=|-[a-zA-Z]+)$"))

;; #F29668 #000000 0 0 0 1
[
  "not"
  "and"
  "or"
] @keyword.operator

;; #FF8F40 #000000 0 0 0 1
(if_statement
  [
    "if"
    "end"
  ] @keyword.conditional)

;; #FF8F40 #000000 0 0 0 1
(switch_statement
  [
    "switch"
    "end"
  ] @keyword.conditional)

;; #FF8F40 #000000 0 0 0 1
(case_clause
  "case" @keyword.conditional)

;; #FF8F40 #000000 0 0 0 1
(else_clause
  "else" @keyword.conditional)

;; #FF8F40 #000000 0 0 0 1
(else_if_clause
  [
    "else"
    "if"
  ] @keyword.conditional)

; Loops/Blocks
;; #FF8F40 #000000 0 0 0 1
(while_statement
  [
    "while"
    "end"
  ] @keyword.repeat)

;; #FF8F40 #000000 0 0 0 1
(for_statement
  [
    "for"
    "end"
  ] @keyword.repeat)

;; #FF8F40 #000000 0 0 0 1
(begin_statement
  [
    "begin"
    "end"
  ] @keyword.repeat)

; Keywords
;; #FF8F40 #000000 0 0 0 1
[
  "in"
  (break)
  (continue)
] @keyword

;; #FF8F40 #000000 0 0 0 1
"return" @keyword.return

;; #BFBDB6 #000000 0 0 0 1
[
  "["
  "]"
  "{"
  "}"
  "("
  ")"
] @punctuation.bracket

;; #BFBDB6 #000000 0 0 0 1
"," @punctuation.delimiter

;; #7dcfff #000000 0 0 0 2
(command_substitution
  "$" @punctuation.special)

;; #FFB454 #000000 0 0 0 3
(command
  name: (word) @function.call)

;; #FFB454 #000000 0 0 0 3
(command
  name: (word) @function.builtin
  (#match? @function.builtin
    "^(\\.|:|_|abbr|alias|argparse|bg|bind|block|breakpoint|builtin|cd|command|commandline|complete|contains|count|disown|echo|emit|eval|exec|exit|fg|functions|history|isatty|jobs|math|path|printf|pwd|random|read|realpath|set|set_color|source|status|string|test|time|type|ulimit|wait)$"))

;; #FF8F40 #000000 0 0 0 1
(function_definition
  [
    "function"
    "end"
  ] @keyword.function)

;; #FFB454 #000000 0 0 0 3
(function_definition
  name: [
    (word)
    (concatenation)
  ] @function)

;; #FFFFFF #000000 0 0 0 1
(function_definition
  option: [
    (word)
    (concatenation
      (word))
  ] @variable.parameter
  (#match? @variable.parameter "^[-]"))

;; #AAD94C #000000 0 0 0 1
[
  (double_quote_string)
  (single_quote_string)
] @string

;; #AAD94C #000000 0 0 0 1
(escape_sequence) @string.escape

;; #FFFFFF #000000 0 0 0 1
(variable_name) @variable

;; #D2A6FF #000000 0 0 0 1
(variable_expansion) @constant

;; #7dcfff #000000 0 0 0 2
(variable_expansion
  "$" @punctuation.special) @none

;; #F07178 #000000 0 0 0 1
((variable_name) @variable.builtin
  (#match? @variable.builtin
    "^(PATH|CDPATH|LANG|LC_ALL|LC_COLLATE|LC_CTYPE|LC_MESSAGES|LC_MONETARY|LC_NUMERIC|LC_TIME|fish_color_normal|fish_color_command|fish_color_keyword|fish_color_redirection|fish_color_end|fish_color_error|fish_color_param|fish_color_valid_path|fish_color_option|fish_color_comment|fish_color_selection|fish_color_operator|fish_color_escape|fish_color_autosuggestion|fish_color_cwd|fish_color_cwd_root|fish_color_user|fish_color_host|fish_color_host_remote|fish_color_status|fish_color_cancel|fish_color_search_match|fish_color_history_current|fish_pager_color_progress|fish_pager_color_background|fish_pager_color_prefix|fish_pager_color_completion|fish_pager_color_description|fish_pager_color_selected_background|fish_pager_color_selected_prefix|fish_pager_color_selected_completion|fish_pager_color_selected_description|fish_pager_color_secondary_background|fish_pager_color_secondary_prefix|fish_pager_color_secondary_completion|fish_pager_color_secondary_description|fish_term24bit|fish_term256|fish_ambiguous_width|fish_emoji_width|fish_autosuggestion_enabled|fish_handle_reflow|fish_key_bindings|fish_escape_delay_ms|fish_sequence_key_delay_ms|fish_complete_path|fish_cursor_selection_mode|fish_cursor_default|fish_cursor_insert|fish_cursor_replace|fish_cursor_replace_one|fish_cursor_visual|fish_cursor_external|fish_function_path|fish_greeting|fish_history|fish_trace|FISH_DEBUG|FISH_DEBUG_OUTPUT|fish_user_paths|umask|BROWSER|_|argv|CMD_DURATION|COLUMNS|LINES|fish_kill_signal|fish_killring|fish_read_limit|fish_pid|history|HOME|hostname|IFS|last_pid|PWD|pipestatus|SHLVL|status|status_generation|TERM|USER|EUID|version|FISH_VERSION)$"))

;; #D2A6FF #000000 0 0 0 2
[
  (integer)
  (float)
] @number

;; #99ADBF #000000 0 1 0 1
(comment) @comment

;; #99ADBF #000000 0 1 0 1
(comment) @spell

;; #D2A6FF #000000 0 0 0 1
((word) @boolean
  (#match? @boolean "^(true|false)$"))

;; #AAD94C #000000 0 0 0 3
((program
  .
  (comment) @keyword.directive @nospell)
  (#match? @keyword.directive "^#!"))

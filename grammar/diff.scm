;; #99ADBF #000000 0 1 0 0 1
(comment) @comment @spell

;; #A6E3A1 #000000 0 0 0 0 2
(addition) @diff.plus

;; #F07178 #000000 0 0 0 0 2
(deletion) @diff.minus

;; #D2A6FF #000000 0 0 0 0 0
[
  (new_file)
  (old_file)
] @file

;; #D2A6FF #000000 0 0 0 0 1
(commit) @constant

;; #7dcfff #000000 0 0 0 0 2
(location) @attribute

;; #D2A6FF #000000 0 0 0 0 1
(command
  "diff" @function
  (argument) @variable.parameter)

;; #7dcfff #000000 0 0 0 0 6
(mode) @number

;; #888888 #000000 0 0 0 0 3
[
  ".."
  "+"
  "++"
  "+++"
  "++++"
  "-"
  "--"
  "---"
  "----"
] @punctuation.special

;; #7dcfff #000000 0 0 0 0 2
[
  (binary_change)
  (similarity)
  (file_change)
] @label

;; #D2A6FF #000000 0 0 0 0 1
(index
  "index" @keyword)

;; #FF8F40 #000000 0 0 0 0 1
(similarity
  (score) @number
  "%" @number)

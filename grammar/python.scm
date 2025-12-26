; ============================================================
; Identifiers
; ============================================================

;; #FFFFFF #000000 0 0 0 1
(identifier) @variable

;; #D2A6FF #000000 0 0 0 2
((identifier) @type
  (#match? @type "^[A-Z].*[a-z]"))

;; #D2A6FF #000000 0 0 0 2
((identifier) @constant
  (#match? @constant "^[A-Z][A-Z_0-9]*$"))

;; #D2A6FF #000000 0 0 0 2
((identifier) @constant.builtin
  (#match? @constant.builtin "^__[a-zA-Z0-9_]*__$"))

;; #D2A6FF #000000 0 0 0 2
((identifier) @constant.builtin
  (#match? @constant.builtin "^(NotImplemented|Ellipsis|quit|exit|copyright|credits|license)$"))

;; #FFB454 #000000 0 0 0 3
((assignment
  left: (identifier) @type.definition
  (type
    (identifier) @_annotation))
  (#match? @_annotation "^TypeAlias$"))

;; #FFB454 #000000 0 0 0 3
((assignment
  left: (identifier) @type.definition
  right: (call
    function: (identifier) @_func))
  (#match? @_func "^(TypeVar|NewType)$"))

; ============================================================
; Function definitions
; ============================================================

;; #FFB454 #000000 0 0 0 3
(function_definition
  name: (identifier) @function)

;; #FFB454 #000000 0 0 0 2
(type
  (identifier) @type)

;; #FFB454 #000000 0 0 0 2
(type
  (subscript
    (identifier) @type))

;; #FFB454 #000000 0 0 0 2
((call
  function: (identifier) @_isinstance
  arguments: (argument_list
    (_)
    (identifier) @type))
  (#match? @_isinstance "^isinstance$"))

; ============================================================
; Literals
; ============================================================

;; #D2A6FF #000000 0 0 0 2
(none) @constant.builtin

;; #D2A6FF #000000 0 0 0 2
[
  (true)
  (false)
] @boolean

;; #D2A6FF #000000 0 0 0 2
(integer) @number

;; #D2A6FF #000000 0 0 0 2
(float) @number.float

;; #99ADBF #000000 0 1 0 1
(comment) @comment @spell

;; #F29668 #000000 0 0 0 1
((module
  .
  (comment) @keyword.directive @nospell)
  (#match? @keyword.directive "^#!/"))

;; #AAD94C #000000 0 0 0 0
(string) @string

;; #AAD94C #000000 0 0 0 0
[
  (escape_sequence)
  (escape_interpolation)
] @string.escape

;; #AAD94C #000000 0 0 0 0
(expression_statement
  (string
    (string_content) @spell) @string.documentation)

; ============================================================
; Operators
; ============================================================

;; #FF8F40 #000000 0 0 0 1
[ "if" "elif" "else" "for" "while" "break" "continue" ] @keyword.control_flow_loops

;; #FF8F40 #000000 0 0 0 1
[ "def" "return" "lambda" "yield" "async" "await" ] @keyword.functions_coroutines

;; #7dcfff #000000 0 0 0 2
[ "class" ] @keyword.class

;; #F07178 #000000 0 0 0 1
[ "try" "except" "finally" "raise" ] @keyword.exceptions

;; #D2A6FF #000000 0 0 0 2
[ "with" ] @keyword.context_management

;; #7dcfff #000000 0 0 0 2
[ "import" "from" "exec" ] @keyword.imports_execution

;; #D2A6FF #000000 0 0 0 2
[ "match" "case" ] @keyword.pattern_matching

;; #F07178 #000000 0 0 0 1
[ "global" "nonlocal" ] @keyword.scope_bindings

;; #FF8F40 #000000 0 0 0 1
[ "del" ] @keyword.deletion

;; #FF8F40 #000000 0 0 0 1
[ "pass" "assert" "as" "print" ] @keyword.utility

;; #F29668 #000000 0 1 0 1
[
  "-"
  "-="
  "!="
  "*"
  "**"
  "**="
  "*="
  "/"
  "//"
  "//="
  "/="
  "&"
  "&="
  "%"
  "%="
  "^"
  "^="
  "+"
  "->"
  "+="
  "<"
  "<<"
  "<<="
  "<="
  "<>"
  "="
  ":="
  "=="
  ">"
  ">="
  ">>"
  ">>="
  "|"
  "|="
  "~"
  "@="
  "and"
  "in"
  "is"
  "not"
  "or"
  "is not"
  "not in"
] @operatoroperator

;; #BFBDB6 #000000 0 0 0 1
[
  ","
  "."
  ":"
  ";"
  (ellipsis)
] @punctuation.delimiter

;; #BFBDB6 #000000 0 0 0 1
[
  "("
  ")"
  "["
  "]"
  "{"
  "}"
] @punctuation.bracket

;; #7dcfff #000000 0 0 0 2
(interpolation
  "{" @punctuation.special
  "}" @punctuation.special)

;; #7dcfff #000000 0 0 0 2
(format_expression
  "{" @punctuation.special
  "}" @punctuation.special)

;; #7dcfff #000000 0 0 0 2
(line_continuation) @punctuation.special

;; #FFB454 #000000 0 0 0 2
(type_conversion) @function.macro

; ============================================================
; Builtins / Exception types
; ============================================================

;; #D2A6FF #000000 0 0 0 2
((identifier) @type.builtin
  (#match? @type.builtin
   "^(BaseException|Exception|ArithmeticError|BufferError|LookupError|AssertionError|AttributeError|EOFError|FloatingPointError|GeneratorExit|ImportError|ModuleNotFoundError|IndexError|KeyError|KeyboardInterrupt|MemoryError|NameError|NotImplementedError|OSError|OverflowError|RecursionError|ReferenceError|RuntimeError|StopIteration|StopAsyncIteration|SyntaxError|IndentationError|TabError|SystemError|SystemExit|TypeError|UnboundLocalError|UnicodeError|UnicodeEncodeError|UnicodeDecodeError|UnicodeTranslateError|ValueError|ZeroDivisionError|EnvironmentError|IOError|WindowsError|BlockingIOError|ChildProcessError|ConnectionError|BrokenPipeError|ConnectionAbortedError|ConnectionRefusedError|ConnectionResetError|FileExistsError|FileNotFoundError|InterruptedError|IsADirectoryError|NotADirectoryError|PermissionError|ProcessLookupError|TimeoutError|Warning|UserWarning|DeprecationWarning|PendingDeprecationWarning|SyntaxWarning|RuntimeWarning|FutureWarning|ImportWarning|UnicodeWarning|BytesWarning|ResourceWarning)$"))

; ============================================================
; Function / Lambda parameters
; ============================================================

;; #D2A6FF #000000 0 0 0 1
(parameters
  (identifier) @variable.parameter)

;; #D2A6FF #000000 0 0 0 1
(lambda_parameters
  (identifier) @variable.parameter)

;; #D2A6FF #000000 0 0 0 1
(lambda_parameters
  (tuple_pattern
    (identifier) @variable.parameter))

;; #D2A6FF #000000 0 0 0 1
(keyword_argument
  name: (identifier) @variable.parameter)

;; #D2A6FF #000000 0 0 0 1
(default_parameter
  name: (identifier) @variable.parameter)

;; #D2A6FF #000000 0 0 0 1
(typed_parameter
  (identifier) @variable.parameter)

;; #D2A6FF #000000 0 0 0 1
(typed_default_parameter
  name: (identifier) @variable.parameter)

;; #D2A6FF #000000 0 0 0 1
(parameters
  (list_splat_pattern
    (identifier) @variable.parameter))

;; #D2A6FF #000000 0 0 0 1
(parameters
  (dictionary_splat_pattern
    (identifier) @variable.parameter))

;; #D2A6FF #000000 0 0 0 1
(lambda_parameters
  (list_splat_pattern
    (identifier) @variable.parameter))

;; #D2A6FF #000000 0 0 0 1
(lambda_parameters
  (dictionary_splat_pattern
    (identifier) @variable.parameter))

;; #FFB454 #000000 0 0 0 2
((identifier) @variable.builtin
  (#match? @variable.builtin "^(self|cls)$"))

; ============================================================
; Attributes / Class members
; ============================================================

;; #FFB454 #000000 0 0 0 2
((attribute
  attribute: (identifier) @variable.member)
  (#match? @variable.member "^[%l_].*$"))

; ============================================================
; Class definitions
; ============================================================

;; #59C2FF #000000 0 0 0 2
(class_definition
  name: (identifier) @type)

;; #FFB454 #000000 0 0 0 2
(class_definition
  body: (block
    (function_definition
      name: (identifier) @function.method)))

;; #D2A6FF #000000 0 0 0 2
(class_definition
  superclasses: (argument_list
    (identifier) @type))

;; #FFB454 #000000 0 0 0 2
((class_definition
  body: (block
    (expression_statement
      (assignment
        left: (identifier) @variable.member))))
  (#match? @variable.member "^[%l_].*$"))

;; #FFB454 #000000 0 0 0 2
((class_definition
  body: (block
    (expression_statement
      (assignment
        left: (_
          (identifier) @variable.member)))))
  (#match? @variable.member "^[%l_].*$"))

;; #FFB454 #000000 0 0 0 2
((class_definition
  (block
    (function_definition
      name: (identifier) @constructor)))
  (#match? @constructor "^(__new__|__init__)$"))

; ============================================================
; Function calls
; ============================================================

;; #FFB454 #000000 0 0 0 2
(call
  function: (identifier) @function.call)

;; #FFB454 #000000 0 0 0 2
(call
  function: (attribute
    attribute: (identifier) @function.method.call))

;; #59C2FF #000000 0 0 0 3
((call
  function: (identifier) @constructor)
  (#match? @constructor "^[A-Z]"))

;; #59C2FF #000000 0 0 0 3
((call
  function: (attribute
    attribute: (identifier) @constructor))
  (#match? @constructor "^[A-Z]"))

;; #FFB454 #000000 0 0 0 2
((call
  function: (identifier) @function.builtin)
  (#match? @function.builtin
   "^(abs|all|any|ascii|bin|bool|breakpoint|bytearray|bytes|callable|chr|classmethod|compile|complex|delattr|dict|dir|divmod|enumerate|eval|exec|filter|float|format|frozenset|getattr|globals|hasattr|hash|help|hex|id|input|int|isinstance|issubclass|iter|len|list|locals|map|max|memoryview|min|next|object|oct|open|ord|pow|print|property|range|repr|reversed|round|set|setattr|slice|sorted|staticmethod|str|sum|super|tuple|type|vars|zip|__import__)$"))

; ============================================================
; Regex call
; ============================================================

(call
  function: (identifier) @_re
  arguments: (argument_list
;; !regex
    (string) @string.regexp
  )
  (#match? @_re "re"))

; ============================================================
; Decorators
; ============================================================

;; #FFB454 #000000 0 0 0 2
(decorator
  "@" @attribute)

;; #FFB454 #000000 0 0 0 2
(decorator
  (identifier) @attribute)

;; #FFB454 #000000 0 0 0 2
(decorator
  (attribute
    attribute: (identifier) @attribute))

;; #FFB454 #000000 0 0 0 2
(decorator
  (call
    (identifier) @attribute))

;; #FFB454 #000000 0 0 0 2
(decorator
  (call
    (attribute
      attribute: (identifier) @attribute)))

;; #59C2FF #000000 0 0 0 3
((decorator
  (identifier) @attribute.builtin)
  (#match? @attribute.builtin "^(classmethod|property|staticmethod)$"))

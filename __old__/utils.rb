Fold = Struct.new(:start, :end, :active, :crc32)
Highlight = Struct.new(:color_fg, :color_bg, :flags, :priority)
Token = Struct.new(:start, :end, :sym)

CF_NONE      = 0b00000000
CF_ITALIC    = 0b00000001
CF_BOLD      = 0b00000010
CF_UNDERLINE = 0b00000100

def ctrl_key(k)
  k.ord & 0x1F
end

# Key types
KEY_TYPE = {
  0 => :char,
  1 => :special,
  2 => :mouse,
  3 => :none
}.freeze

# Special keys
SPECIAL_KEY = {
  0 => :up,
  1 => :down,
  2 => :left,
  3 => :right,
  4 => :delete
}.freeze

# Control key
KEY_ESC = "\x1b".freeze

# Mouse states
MOUSE_STATE = {
  0 => :press,
  1 => :release,
  2 => :drag,
  3 => :scroll
}.freeze

# Mouse buttons
MOUSE_BUTTON = {
  0 => :left,
  1 => :middle,
  2 => :right,
  3 => :scroll,
  4 => :none
}.freeze

# Scroll directions
SCROLL_DIR = {
  0 => :up,
  1 => :down,
  2 => :left,
  3 => :right,
  4 => :none
}.freeze

# Modifiers
MODIFIER = {
  0 => :none,
  1 => :alt,
  2 => :cntrl,
  3 => :cntrl_alt,
  4 => :shift
}.freeze

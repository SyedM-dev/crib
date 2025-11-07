def ctrl_key(k)
  k.ord & 0x1F
end

# Key types
KEY_TYPE = {
  0 => :char,
  1 => :special,
  2 => :mouse
}

# Special keys
SPECIAL_KEY = {
  0 => :up,
  1 => :down,
  2 => :left,
  3 => :right,
  4 => :delete
}

# Control key
KEY_ESC = "\x1b"

# Mouse states
MOUSE_STATE = {
  0 => :press,
  1 => :release,
  2 => :drag,
  3 => :scroll
}

# Mouse buttons
MOUSE_BUTTON = {
  0 => :left,
  1 => :middle,
  2 => :right,
  3 => :scroll,
  4 => :none
}

# Scroll directions
SCROLL_DIR = {
  0 => :up,
  1 => :down,
  2 => :left,
  3 => :right,
  4 => :none
}

# Modifiers
MODIFIER = {
  1 => :alt,
  2 => :cntrl,
  3 => :cntrl_alt,
  4 => :shift
}

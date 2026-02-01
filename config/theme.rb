# this can be modified by the user during runtime through keybindings
# But i need to know how to ever read this value only when needed.
# maybe i can write a function that notifies if theme maybe changed then reload
# It can also be scripted to load different theme formats into a hash usable by crib
C.theme = {
  :default => { fg: 0xEEEEEE },
  :shebang => { fg: 0x7DCFFF },
  :error => { fg: 0xEF5168 },
  :comment => { fg: 0xAAAAAA, italic: true },
  :string => { fg: 0xAAD94C },
  :escape => { fg: 0x7DCFFF },
  :interpolation => { fg: 0x7DCFFF },
  :regexp => { fg: 0xD2A6FF },
  :number => { fg: 0xE6C08A },
  # rubocop:disable Lint/BooleanSymbol
  :true => { fg: 0x7AE93C },
  :false => { fg: 0xEF5168 },
  # rubocop:enable Lint/BooleanSymbol
  :char => { fg: 0xFFAF70 },
  :keyword => { fg: 0xFF8F40 },
  :keywordoperator => { fg: 0xF07178 },
  :operator => { fg: 0xFFFFFF, italic: true },
  :function => { fg: 0xFFAF70 },
  :type => { fg: 0xF07178 },
  :constant => { fg: 0x7DCFFF },
  :variableinstance => { fg: 0x95E6CB },
  :variableglobal => { fg: 0xF07178 },
  :annotation => { fg: 0x7DCFFF },
  :directive => { fg: 0xFF8F40 },
  :label => { fg: 0xD2A6FF },
  :brace1 => { fg: 0xD2A6FF },
  :brace2 => { fg: 0xFFAFAF },
  :brace3 => { fg: 0xFFFF00 },
  :brace4 => { fg: 0x0FFF0F },
  :brace5 => { fg: 0xFF0F0F }
}

# basic configuration
# This can also be used to do speacail configs for different projects.
# its ruby guys script whatever you want.

C.startup do
  puts "Starting crib..."
end

C.shutdown do
  puts "Exiting crib..."
end

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

# # TODO: to be done once a proper api for binding and window drawing is made
# #       The binds will be connected to either `editor` or windows where editor can
# #       only use a preset set of stuff to bind while teh windows are purely custom
# # # this part uses dsl bindings to define the bind function
# # # Hopefully extend to give more context/power to bindings
# # # but try to keep simple for performance
# # # for default keybindings
# # C.bind [:normal, :select], :a => "insert_mode"
# # # for custom keybindings
# # C.bind :select, [:x, :c] do
# #   puts "cut"
# # end
# # C.bind :jumper do
# #   set [:x, :c] do
# #     puts "jump to first bookmark"
# #   end
# # end
# # # they can also be defined conditionally
# # # This code is just an example and doesnt actually work
# # if using_tmux?
# #   bind :C-p do
# #     system("tmux select-pane -U")
# #   end
# # end

# This can, for example, be modified by user bindings during runtime
# TODO: dynamic registration to actually be implemented once keybinds and extentions are implemented
# A predefined list already exists and can be found in libcrib.rb
# C.lsp_config["solargraph"] = ["stdio"]
#
# C.languages[:ruby] = {
#   color: 0xff8087,
#   symbol: "󰴭 ",
#   extensions: ["rb"],
#   filenames: ["Gemfile"],
#   mimetypes: ["text/x-ruby"],
#   lsp: "solargraph"
# }

C.line_endings = :auto_unix # or :unix or :windows or :auto_windows

# C.extra_highlights do |_line, _idx|
#   # the return can be an array of
#   # [fg, bg. flags, start, end]
#   # where fg and bg are integers (using 24 bit color)
#   # and flags is a bitmask of bold/underline/italic etc
#   # and start and end are integers strictly inside the line
#   return []
# end

# The highlighter will be aplied to the language as long as the langauge is defined in C.languages
C.highlighters[:ruby_n] = {
  parser: ->(line, state, line_idx) {
    # the return value is a hash
    # it contains the state and the highlights
    # state can be of any type but will be consistent between calls
    # initially nil is sent for uninitialized state the returned must be anything but nil
    # the same state can be used for multiple lines
    # the highlights can be an array of
    # [K_type, start, end]
    # K_type is a constant from the constants defined in libcrib.rb
    # for ex: for strings it would be Tokens::K_STRING or for numbers Tokens::K_NUMBER etc.
    # and start and end are integers strictly inside the line
    return {
      state: "",
      tokens: [
        # This will highlight the entire line as a string
        # Any wrong format will not be handled and lead to crashes
        { type: Tokens::K_STRING, start: 0, end: line.length }
      ]
    }
  },
  matcher: ->(state1, state2) {
    # returns true if the states are equal
    # And so would not need recomputation for further lines
    return state1 == state2
  }
}

# Files can be insluded using Kernel#require_relative
# but it can be called with binding as the second argument
# skipping it will call it with global binding which is usually fine
# Kernel#load can also be used

require_relative "theme"

# basic configuration
# This can also be used to do speacail configs for different projects.
# its ruby guys script whatever you want.

# puts "Loading main config..."

C.startup do
  puts "Starting crib..."
end

C.shutdown do
  puts "Exiting crib..."
end

# TODO: to be done once a proper api for binding and window drawing is made
#       The binds will be connected to either `editor` or windows where editor can
#       only use a preset set of stuff to bind while teh windows are purely custom
# # this part uses dsl bindings to define the bind function
# # Hopefully extend to give more context/power to bindings
# # but try to keep simple for performance
# # for default keybindings
# C.bind [:normal, :select], :a => "insert_mode"
# # for custom keybindings
# C.bind :select, [:x, :c] do
#   puts "cut"
# end
# C.bind :jumper do
#   set [:x, :c] do
#     puts "jump to first bookmark"
#   end
# end
# # they can also be defined conditionally
# # This code is just an example and doesnt actually work
# if using_tmux?
#   bind :C-p do
#     system("tmux select-pane -U")
#   end
# end

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
#   lsp: "solargraph"
# }

C.line_endings = :auto_unix # or :unix or :windows or :auto_windows

C.extra_highlights do |_line, _idx|
  # the return can be an array of
  # [fg, bg. flags, start, end]
  # where fg and bg are integers (using 24 bit color)
  # and flags is a bitmask of bold/underline/italic etc
  # and start and end are integers strictly inside the line
  return []
end

# The highlighter will be aplied to the language as long as the langauge is defined in C.languages
C.highlighters[:string] = {
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

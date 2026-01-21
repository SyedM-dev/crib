module C
  attr_accessor :theme, :lsp_config, :languages,
                :line_endings, :utf_mode, :highlighters
  attr_reader :b_startup, :b_shutdown, :b_extra_highlights

  @lsp_config = {}
  @languages = {}
  @key_handlers = {}
  @key_binds = {}

  def startup(&block)
    @b_startup = block
  end

  def shutdown(&block)
    @b_shutdown = block
  end

  def extra_highlights(&block)
    @b_extra_highlights = block
  end

  def bind(modes, keys = nil, action = nil, &block)
    modes = [modes] unless modes.is_a?(Array)
    if keys.nil?
      app = self
      dsl = Object.new
      dsl.define_singleton_method(:set) do |k, act = nil, &blk|
        app.bind(modes, k, act, &blk)
      end
      dsl.instance_exec(&handler)
    elsif block_given?
      keys = [keys] unless keys.is_a?(Array)
      modes.each do |mode|
        keys.each do |key|
          @key_handlers[mode] ||= {}
          @key_handlers[mode][key] ||= []
          @key_handlers[mode][key] << block
        end
      end
    elsif action.is_a?(String)
      keys = [keys] unless keys.is_a?(Array)
      modes.each do |mode|
        keys.each do |key|
          @key_binds[mode] ||= {}
          @key_binds[mode][key] ||= []
          @key_binds[mode][key] << action
        end
      end
    else
      raise ArgumentError("invalid arguments")
    end
  end
end

# basic configuration

C.startup do
  do_something_random_here!
end

C.shutdown do
  do_something_random_here!
end

# this can be modified by the user during runtime through keybindings
# But i need to know how to ever read this value only when needed .
# maybe i can write a function that notifies if theme maybe changed then reload
C.theme = {
  # i have a predefined list of keys that can be used here
  :default => {
    # here fg bg and style are all optional and have default values
    # if not specified
    fg: 0xEEEEEE,
    bg: 0x000000,
    italic: false,
    bold: false,
    underline: false,
    strikethrough: false
  }
}

# this part uses dsl bindings to define the bind function
# Hopefully extend to give more context/power to bindings
# but try to keep simple for performance
# for default keybindings
C.bind [:normal, :select], :a => "insert_mode"
# for custom keybindings
C.bind :select, [:x, :c] do
  puts "cut"
end
C.bind :jumper do
  set [:x, :c] do
    puts "jump to first bookmark"
  end
end
# they can also be defined conditionally
# This code is just an example and doesnt actually work
if using_tmux?
  bind :C-p do
    system("tmux select-pane -U")
  end
end

# This can for example be modified by user bindings during runtime
C.lsp_config[:solargraph] = {
  command: "solargraph",
  args: ["stdio"],
  languages: [:ruby]
}

# these are actually cached into cpp by the editor upon setting
C.languages[:ruby] = {
  color: 0xff8087,
  symbol: "󰴭 ",
  extensions: ["rb"],
  filenames: ["Gemfile"],
  mimetypes: ["text/x-ruby"]
}

C.line_endings = :auto_unix # or :auto_windows or :unix or :windows to force
C.utf_mode = :auto_utf8 # or :auto_utf16 or :utf8 or :utf16 to force

C.extra_highlights do |_line, _idx|
  # the return can be an array of
  # [fg, bg. flags, start, end]
  # where fg and bg are integers (using 24 bit color)
  # and flags is a bitmask of bold/underline/italic etc
  # and start and end are integers strictly inside the line
  return []
end

C.highlighters[:language_name] = {
  parser: ->(_state, _line) {
    # the return value is an array of
    # [state, highlights]
    # state can be of any type but will be consistent between calls
    # initially nil is sent for uninitialized state
    # the highlights can be an array of
    # [fg, bg. flags, start, end]
    # where fg and bg are integers (using 24 bit color)
    # and flags is a bitmask of bold/underline/italic etc
    # and start and end are integers strictly inside the line
    return []
  },
  matcher: ->(_state1, _state2) {
    # returns true if the states are equal
    # And so would not need recomputation for further lines
    return false
  }
}

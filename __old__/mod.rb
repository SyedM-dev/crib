module C
  extend FFI::Library
  ffi_lib File.join(__dir__, "../../libs/tree-sitter/libtree-sitter.so")

  # TSTree and TSParser are opaque pointers
  typedef :pointer, :tstree
  typedef :pointer, :tsparser

  # Define TSNode struct
  class TSNode < FFI::Struct
    layout :context, [:uint32, 4],
           :id, :pointer,
           :tree, :tstree
  end

  class TSPoint < FFI::Struct
    layout :row, :uint32,
           :column, :uint32
  end

  class TSInputEdit < FFI::Struct
    layout :start_byte,      :uint32,
           :old_end_byte,    :uint32,
           :new_end_byte,    :uint32,
           :start_point,     TSPoint.by_value,
           :old_end_point,   TSPoint.by_value,
           :new_end_point,   TSPoint.by_value
  end

  class TSTreeCursor < FFI::Struct
    layout :tree, :pointer,
           :id, :pointer,
           :context, [:uint32, 3]
  end

  attach_function :ts_parser_new, [], :tsparser
  attach_function :ts_parser_set_language, [:tsparser, :pointer], :bool
  attach_function :ts_parser_parse_string, [:tsparser, :tstree, :string, :size_t], :tstree, blocking: true
  attach_function :ts_tree_delete, [:tstree], :void

  attach_function :ts_node_start_byte, [TSNode.by_value], :uint32
  attach_function :ts_node_end_byte, [TSNode.by_value], :uint32
  attach_function :ts_node_symbol, [TSNode.by_value], :uint16

  attach_function :ts_tree_edit, [:tstree, TSInputEdit.by_ref], :void

  attach_function :ts_tree_root_node, [:tstree], TSNode.by_value, blocking: true

  attach_function :ts_node_child_count, [TSNode.by_value], :uint32
  attach_function :ts_node_child, [TSNode.by_value, :uint32], TSNode.by_value



  # Ruby grammar
  ffi_lib File.join(__dir__, "../../libs/tree-sitter-ruby/libtree-sitter-ruby.so")
  attach_function :tree_sitter_ruby, [], :pointer


  ffi_lib File.join(__dir__, "../../builds/C-crib.so")

  class KeyEvent < FFI::Struct
    layout :key_type, :uint8,
           :c, :char,
           :special_key, :uint8,
           :special_modifier, :uint8,
           :mouse_x, :uint8,
           :mouse_y, :uint8,
           :mouse_button, :uint8,
           :mouse_state, :uint8,
           :mouse_direction, :uint8,
           :mouse_modifier, :uint8

    def to_s
      case KEY_TYPE[self[:key_type]]
      when :char
        "#<KeyEvent char=#{self[:c].inspect}>"
      when :special
        "#<KeyEvent special key=#{SPECIAL_KEY[self[:special_key]]} mod=#{MODIFIER[self[:special_modifier]]}>"
      when :mouse
        "#<KeyEvent mouse x=#{self[:mouse_x]} y=#{self[:mouse_y]} " \
        "btn=#{MOUSE_BUTTON[self[:mouse_button]]} state=#{MOUSE_STATE[self[:mouse_state]]} " \
        "scroll_dir=#{SCROLL_DIR[self[:mouse_direction]]} mod=#{MODIFIER[self[:mouse_modifier]]}>"
      else
        "#<KeyEvent type=#{self[:key_type]} unknown=true>"
      end
    end
  end

  class Coords < FFI::Struct
    layout :x, :int,
           :y, :int

    def to_ary
      [self[:x], self[:y]]
    end

    def to_s
      "#<Coords x=#{self[:x]} y=#{self[:y]}>"
    end
  end

  class Size < FFI::Struct
    layout :rows, :int,
           :cols, :int

    def to_ary
      [self[:rows], self[:cols]]
    end

    def to_s
      "#<Size rows=#{self[:rows]} cols=#{self[:cols]}>"
    end
  end

  attach_function :load_query, [:string], :void
  attach_function :ts_collect_tokens, [TSNode.by_value, :pointer, :pointer, :string], :pointer, blocking: true
  attach_function :free_tokens, [:pointer], :void, blocking: true
  attach_function :start_screen, [], Size.by_value
  attach_function :end_screen, [], :void
  attach_function :update, [:int, :int, :string, :uint32, :uint32, :uint8], :void
  attach_function :render, [], :void, blocking: true
  attach_function :set_cursor, [:int, :int, :int], :void
  attach_function :read_key, [], KeyEvent.by_value, blocking: true
  attach_function :get_size, [], Coords.by_value
  attach_function :display_width, [:string], :int, blocking: true
end

require "ffi"
require_relative "utils"

module C
  extend FFI::Library
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

  attach_function :start_screen, [], :void
  attach_function :end_screen, [], :void
  attach_function :update, [:int, :int, :string, :uint32, :uint32, :uint8], :void
  attach_function :render, [], :void
  attach_function :set_cursor, [:int, :int, :int], :void
  attach_function :read_key, [], KeyEvent.by_value
  attach_function :get_size, [], Coords.by_value
  attach_function :real_width, [:string], :int
end

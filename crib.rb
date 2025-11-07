#!/usr/bin/env ruby

require_relative "./src/ruby/mod"

# C.start_screen
#
# at_exit do
#   C.end_screen
#   puts "bye"
# end
#
#
# class Tester
#   @current_row = 0
#
#   class << self
#     def debug_print(text)
#       text.each_char.with_index do |ch, i|
#         C.update(@current_row, i, ch, 0xFFFFFE, 0x000001, 0)
#       end
#       @current_row += 1
#     end
#
#     def reset
#       @current_row = 0
#     end
#   end
# end
#
# render_thread = Thread.new do
#   loop do
#     sleep(1.0/20)
#     C.render
#   end
# end
#
# loop do
#   sleep 0.001
#   event = C.read_key
#   break if event[:key_type] == 0 && event[:c] == 'q'.ord
#   Tester.reset if event[:key_type] == 0 && event[:c] == 'r'.ord
#   Tester.debug_print(C.get_size.to_s) if event[:key_type] == 0 && event[:c] == 's'.ord
#   Tester.debug_print(event.to_s)
# end
#
# render_thread.kill
# render_thread.join

require_relative("./src/ruby/editor.rb")

test = Buffer.new("hello world\nwow")

pp test.render_text(0, 0, 10, 10)

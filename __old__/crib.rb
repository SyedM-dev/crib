#!/usr/bin/env ruby

require "ffi"
require "zlib"

require_relative "../src/ruby/utils"
require_relative "../src/ruby/mod"
require_relative "../src/ruby/ts_rb"
require_relative "../src/ruby/fm"
require_relative "../src/ruby/editor"
require_relative "../src/ruby/ide"

$rows, $cols = C.start_screen
$running = true
$event_queue = Queue.new
$folder = Dir.new File.dirname(ARGV[0] || Dir.pwd)
$threads = []

at_exit do
  IDE.close
  C.end_screen
  puts "Exiting crib.rb"
end

IDE.start

$threads << Thread.new do
  loop do
    sleep 1.0 / 64
    break unless $running
    IDE.handle_event $event_queue.pop timeout: 0 until $event_queue.empty?
    IDE.render
    C.render
  end
end

$threads << Thread.new do
  loop do
    sleep 1.0 / 64
    break unless $running
    IDE.work!
  end
end

$threads << Thread.new do
  loop do
    break unless $running
    event = C.read_key # read_key is blocking
    $running = false if KEY_TYPE[event[:key_type]] == :char && event[:c] == ctrl_key('q')
    $event_queue << event
  end
end

$threads.each &:join

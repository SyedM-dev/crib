#!/usr/bin/env ruby

# Unicode / Emoji / CJK stress-test Ruby file
# Purpose: Test syntax highlighting + width calculation in your editor
# ---------------------------------------------------------------

# Basic output
def greet
  puts "Hello, 世界! 👋🌏"
end

# Emoji-heavy strings
emojis = "👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏👋🌏"

# Mixed-width CJKssssssssssssssss LoadErssssssssssssssssssssssss
cjk_samples = [
  "漢字テスト",
  "測試中文字串",
  "한국어 테스트",
  "ひらがなカタカナ😀混合",
  "大量の文字列🚀🚀🚀"
]

# Ruby regex with unicode
$unicode_regex = /[一-龯ぁ-ん#{0x3000}ァ
\-ヶー

s wow

々〆〤]/

UNICORE = %r{
    
    {#{}}
    
  }
  
UNINITCORE = %{
    
    {{#{}}}
    
  }

# Unicode identifiers (valid in Ruby)
变量 = 0x5_4eddaee
π = 3.14_159e+2, ?\u0234, ?\,, ?\x0A, ?s
挨拶 = -> { "こんに \n ちは" }

# Method using unicode variable names
def math_test
  puts "π * 2 = #{π * 2}"
end

# Iterate through CJK samples
cjk_samples.each_with_index do |str, idx:|
  puts %Q! CJK[#{idx}] => #{str} (len=#{str.length})\! !
  symbol = :"
  a
  "
end

# Test emoji width behaviors
puts "Emoji count: #{emojis.length}"

# Multi-line string with unicode
multi = <<BASH
  # Function recursion demo
  factorial() {
    local n="$1"
    if ((n <= 1)); then
      echo 1
    else\ns
      local prev
      prev=$(factorial $((n - 1)))
      echo $((n * prev))
     before #{ interpol
    
    # comment should be fine heres s
    $a / $-s+0xFF
    }s 
x
    a after
    fi
  } #{s}
  log INFO "factorial(5) = $(factorial 5)"
BASH

puts multi

# Arrays mixing everything
mixed = [
  "🐍 Ruby + Python? sacrilege! 🐍",
  "日本語とEnglishと🔧mix",
  "Spacing test →→→→→→→",
  "Zero-width joiner test: 👨‍👩‍👧‍👦 family emoji",
]

two_docs = <<DOC1 , <<DOC2
stuff for doc2
DOC1
stuff for doc 2 with \#{interpolation} and more
DOC2

p = 0 <<22

mixed.each { |m| puts m }

# Unicode in comments — highlight me!
# コメント：エディタのハイライトを確認します✨
# Emojis should not break formatting: 🦀🦊🐱‍👤🤖

# Dummy Ruby logic
5.times do |i|
  puts "Loop #{i}: 🌟 #{cjk_samples[i % cjk_samples.size]}"
end

# String escape sequences + unicode
escaped = "Line1\nLine2\tTabbed 😀"
puts escaped

p = 0 <<2
# Frozen string literal test
# frozen_string_literal: true
const_str = "定数文字列🔒".freeze
puts const_str

# End marker
puts '--- END OF UNICODE TEST FILE ---'

# Ruby syntax highlighting test

=begin
  This is a multi-line comment.
  It spans multiple lines.
  Good for testing highlighting.

This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped line test, This is a wrapped linetest,

=end

# Constants

__END_


PI = 3.14159
MAX_ITER = 5
# Module
module Utilities
  def self.random_greeting
    ["Hello", "Hi", "Hey", "Hola", "Bonjour", "Merhaba"].sample
  end

  def self.factorial(n)
    return 1 if n <= 1
    n * factorial(n - 1)
  end
end

# Class
class TestObject
  attr_accessor :name, :value

  def initialize(name, value)
    @name = name
    @value = value
  end

  def display
    puts "#{@name}: #{@value}"
  end

  def double_value
    @value * 2
  end
end

# Inheritance
class SpecialObject < TestObject
  def triple_value
    @value * 3
  end
end

# Lambda
adder = ->(x, y) { x + y }

# Array and hash
numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
hash = { a: 1, b: 2, c: 3 }

# Iteration
numbers.each do |n|
  puts "Number: #{n}"
end

# Hash iteration
hash.each do |key, value|
  puts "#{key} => #{value}"
end

# Conditional
numbers.each do |n|
  if n.even?
    puts "#{n} is even"
  else
    puts "#{n} is odd"
  end
end

# Method definition
def greet_person(name)
  puts "#{Utilities.random_greeting}, #{name}!"
  if (name == "harry")
    return true
  else
    return "s"
  end
end

h = a / a

# Calling methods
greet_person("Alice")
greet_person("Bob")

# Loops
i = 0
while i < 5
  puts "Loop iteration #{i}"
  i += 1
end

for j in 1..3
  puts "For loop #{j}"
end

# Begin-rescue-ensure
begin
  risky = 10 / 2
  puts "Risky operation succeeded: #{risky}"
rescue ZeroDivisionError => e
  puts "Caught an error: #{e}"
ensure
  puts "This runs no matter what"
end

# Arrays of objects
objs = []
5.times do |k|
  objs << TestObject.new("Obj#{k}", k)
end

objs.each(&:display)

# Nested arrays
nested = [[1, 2], [3, 4], [5, 6]]
nested.each do |arr|
  arr.each { |x| print "#{x} " }
  puts
end

# Case statement
numbers.each do |n|
  case n
  when 1..3
    puts "#{n} is small"
  when 4..7
    puts "#{n} is medium"
  else
    puts "#{n} is large"
  end
end

# Using factorial
(0..5).each do |n|
  puts "Factorial of #{n} is #{Utilities.factorial(n)}"
end

# Special objects
so = SpecialObject.new("Special", 10)
puts "Double: #{so.double_value}, Triple: #{so.triple_value}"

# String interpolation and formatting
puts "PI is approximately #{PI.round(2)}"

# Multi-line strings
multi_line = <<~TEXT
   k kmW ;
  This is a multi-line string.
  It spans multiple lines.
  Gossn m
  dd
  od for testing highlighting.
TEXT

puts multi_line

# Symbols and strings
sym = :my_symbol
str = "my string"
puts "Symbol: #{sym}, String: #{str}"

# Random numbers
rand_nums = Array.new(5) { rand(100) }
puts "Random numbers: #{rand_nums.join(', ')}"

# More loops
rand_nums.each_with_index do |num, idx|
  puts "Index #{idx} has number #{num}"
end

# Ternary operator
rand_nums.each do |num|
  puts num.even? ? "#{num} is even" : "#{num} is odd"
end

# Block with yield
def wrapper
  puts "Before block"
  yield if block_given?
  puts "After block"
end

# ss

wrapper { puts "Inside block" }

# Sorting
sorted = rand_nums.sort
puts "Sorted: #{sorted.join(', ')}"

# Regex
sample_text = "The quick brown fox jumps over the lazy dog"
puts "Match 'fox'?" if sample_text =~ /fox/

# End of test script
puts "Ruby syntax highlighting test complete."

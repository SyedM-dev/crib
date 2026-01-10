#!/usr/bin/env lua
-- Lua syntax highlighting test file

-- Constants
PI = 3.14159
MAX_COUNT = 100

-- Variables
local counter = 0
local name = "Lua"

-- Built-in variable
print(self)

-- Functions
local function greet(user)
  print("Hello, " .. user)
end

local function add(a, b)
  return a + b
end

-- Method definitions
local obj = {}
function obj:sayHi()
  print("Hi from method!")
end

obj.sayHello = function()
  print("Hello from field function!")
end

-- Arrow-style anonymous function (LuaJIT/CFFI style)
local arrow = function(x)
  return x * 2
end

-- Table constructors
local t = {
  foo = 123,
  bar = function()
    return "bar"
  end,
  nested = {
    a = 1,
    b = 2,
  },
}

-- Loops
for i = 1, MAX_COUNT do
  counter = counter + i
end

while counter > 0 do
  counter = counter - 1
end

repeat
  counter = counter + 1
until counter == 10

-- Conditionals
if counter > 5 then
  print("Big number")
elseif counter == 5 then
  print("Exactly five")
else
  print("Small number")
end

-- Operators
local x, y = 10, 20
local z = x + y * 2 - (x / y) ^ 2
local ok = x == y or x ~= y and not false

-- Function calls
greet("World")
obj:sayHi()
obj.sayHello()
add(5, 10)

-- Built-in function calls
assert(x > 0)
pcall(function()
  print("safe")
end)
tonumber("123")

-- CFFI injection example
local ffi = require("ffi")
ffi.cdef([[
    int printf(const char *fmt, ...);
    typedef struct { int x; int y; } point;
]])

-- Boolean and nil
local flag = true
local nothing = nil

-- Comments
-- Single line
--[[
    Multi-line
    comment
]]

-- Strings
local s1 = "Hello\nWorld"
local s2 = [[Long
multi-line
string]]

-- Template strings (LuaJIT-style)
local tpl = `Value: ${counter}`

-- Regex-like string (for testing injection highlighting)
local re = "/^%a+$/"


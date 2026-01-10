from __future__ import annotations
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Test file for Python Tree-sitter highlighting."""

# ==============================
# Constants / Builtins
# ==============================
PI = 3.14159
MAX_SIZE = 100
NotImplemented
Ellipsis
__name__  # builtin constant

# ==============================
# Imports
# ==============================
import os
import sys as system
from re import compile as re_compile
from math import *

# ==============================
# Functions
# ==============================
def add(a: int, b: int = 5) -> int:
    """Simple add function"""
    return a + b

def variadic(*args, **kwargs):
    print(args, kwargs)

lambda_func = lambda x, y=2: x * y

def type_var_example(T: type):
    pass

# ==============================
# Classes
# ==============================
class Base:
    class_var = 10

    def __init__(self, name: str):
        self.name = name
        self._private = 42

    @classmethod
    def cls_method(cls):
        return cls.class_var

    @staticmethod
    def static_method():
        return "static"

    @property
    def prop(self):
        return self.name

class Derived(Base):
    def __init__(self, name, extra):
        super().__init__(name)
        self.extra = extra

# ==============================
# Variables
# ==============================
normal_var = 1
_local_var = 2
GLOBAL_VAR = 3

# Builtin variable references
self = "something"
cls = "class"

# ==============================
# Control flow
# ==============================
if True:
    x = 10
elif False:
    x = 20
else:
    x = 0

for i in range(3):
    print(i)
while x > 0:
    x -= 1
    if x == 1:
        break
    else:
        continue

try:
    1 / 0
except ZeroDivisionError as err:
    raise
finally:
    pass

# ==============================
# Operators
# ==============================
a, b = 5, 10
c = a + b * 2 // 3 % 4 ** 2
d = (a << 2) & b | c ^ ~a
ef = not a or b and c

# ==============================
# f-strings / interpolation
# ==============================
name = "Alice"
greeting = f"Hello {name.upper()}!"
formatted = f"{a + b} is sum"

# ==============================
# Regex
# ==============================
pattern1 = re_compile(r"\d+")
pattern2 = re_compile(r"\w{2,}")

# ==============================
# Decorators usage
# ==============================
@staticmethod
def static_func():
    return True

@classmethod
def cls_func(cls):
    return cls

# @custom_decorator
def decorated_func():
    return None

# ==============================
# Misc / Type conversions / literals
# ==============================
flag: bool = True
nothing: None = None
num: float = float("3.14")
text: str = str(123)
lst = [1, 2, 3]
tpl = (4, 5)
dct = {"a": 1, "b": 2}

# ==============================
# Type hints / TypeVar / TypeAlias
# ==============================
from typing import TypeVar, NewType
T = TypeVar("T")
UserId = NewType("UserId", int)
TypeAliasExample: type = int

# ==============================
# Function calls / constructors
# ==============================
result = add(1, 2)
obj = Derived("Alice", "extra")
variadic(1, 2, 3, key="value")
instance_check = isinstance(obj, Base)

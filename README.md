# strict_cpython

Currently:
The set of all StrictPython programs is the set of all python3 programs
which don't have unused variables which were declared implicitly.

## Description

This is a fork of cpython.
The original README can be found in [README.rst].

I'm relatively new to python and the lack of explicit declarations disturbs me.
So I do the obvious:
I fork cpython and hammer those checks into this cpython fork.

For example the following typo is not noticed:
```python
def f(x):
    variable = 5
    # ...
    if (x > 1):
        variable = 42
    else:
        variabel = 24 # typo
    return variable
```

The first idea was to use something like perl's `use strict;` and `my`,
but this can't be done without breaking compatibility to python2.

If we only want to stay compatible to python3, we can use (pascal-like) annotated assignments as `my`.
For example:
```python
def f(x):
    variable : int = 5 # Declaration of variable. The type doesn't matter to us (apparently something called "mypy" cares about this).
    # ...
    if (x > 1):
        variable = 42
    else:
        variabel = 24 # Error: variabel was never declared.
    return variable
```

However, we want to stay compatible to python2.
Therefore unused variables which were declared implicitly are made syntax errors.
Because there are a lot of unused variables in existing python code,
you currently have to `import strict_no_unused_vars` to activate the checks.
Example:
```python
import strict_no_unused_vars;

def f(x):
    if (x > 1):
        variable = 42
    else:
        variabel = 24
    return variable
```
StrictPython output:
```
  File "Lib/test/badsyntax_strict_no_unused_vars.py", line 13
    return variable
          ^
SyntaxError: variable 'variabel' defined but never used in unit 'f'
```

If you want to keep your unused variables, you can do one of the following:
```python
def bury(x):
    """ Buries dead assignments."""
    pass

def myfunc2():
    dummy1 = 4
    dummy2 = 8
    del dummy1     # This counts as usage.
    bury(dummy2)   # This counts as usage.
    return 0
```

For classes you don't need such a mechanism.
I thought about introducing lockable dicts or overriding `__setattr__` or `__setitem__`,
but you can just use `__slots__`, if you want to prevent dynamic extension of instance attributes.

## Known issues

StrictPython currently doesn't give `vars()` the special treatment it needs.

Because in Python variable shadowing only happens, if you have nested scopes (i.e. function),
StrictPython doesn't care about shadowing.
If a nested function shadows a variable of the outer function and uses it,
the variable in the outer function is seen as 'used'.

## Future work

The "declaration checks" operate on PyObjects.
Instead of implementing these checks in C they could be implemented in Python:
  - The PyObjects could be made visible to Python.
  - There could be call-outs in the symbol table and compiler functions where Python classes can register.

strict_cpython could then bootstrap its checks from python before compiling other code.
This must only be used for static analysis, because if the extensions lead to different code,
this might conflict with the pycache.


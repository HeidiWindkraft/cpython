# strict_cpython

Currently:
The set of all StrictPython programs is the set of all python3 programs
which don't have unused variables which were declared implicitly.

## Description

This is a fork of cpython.
The original README can be found in [./CPYTHON_README.rst].

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
StrictPython doesn't detect all unused variables, when there are many nested scopes. Use small functions.


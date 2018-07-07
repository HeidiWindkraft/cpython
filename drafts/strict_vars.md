# Package "strict_vars"

Importing `strict_vars` forces explicit declarations for all variables in function scope.
This package is not compatible to Python versions less than 3.6.

## Explicit declarations

Variables are explicitly declared by annotations.
For example:
```python
def f():
    x : int     # Explicitly declares `x`.
    y : int = 2 # Explicitly declares `y`.
    x = 3
    return x * y
```

Declarations must not shadow variables which were already declared in an outer scope.
That's because there is no variable shadowing in Python's functions.
Therefore it is unclear how to treat shadowed variables.
For example:
```python
def f():
    x : int = get_some_xvalue() # Explicitly declares `x`.
    y : int = get_some_yvalue() # Explicitly declares `y`.
    if (x == y):
        x : int = 1;            # Error: "Shadows" previously declared `x`.
        do_something(x,y)
    else:
        do_something_else(x,y)
    return x * y                # What would be the value of `x` here, if `x == y`?
                                # Is it the original `x` or `1`?
                                # In Python it is `1`. In C it would be the original `x`.
```

StrictPython doesn't care about the type given in the annotation.
For example the following code is OK for StrictPython:
```python
def f():
    s : _           # Explicitly declares `s`.
    t : my = 1      # Explicitly declares `t`.
    u : auto = 1    # Explicitly declares `u`.
    v : var = 1     # Explicitly declares `v`.
    w : any = 1     # Explicitly declares `w`.
    x : Any = 1     # Explicitly declares `x`.
    y : [] = 2      # Explicitly declares `y`.
    z : str = 3     # Explicitly declares `z`.
    s = t
    t = u
    u = v
    v = w
    w = x
    x = y
    y = z
    z = x
    return s * t * u * v * w * x * y * z
    # No compile-time error.
```

Using undeclared variables is a compile-time error.
For example:
```python
def f():
    y : int = 2 # Explicitly declares `y`.
    x = 3       # Error: Variable `x` is not declared in this scope.
    return x * y
```

## Declaration contexts

In certain contexts it is assumed that a variable name is a declaration.

### Function parameters

Declarations of function parameters are always explicit declarations.

```python
def f(x,y): # Explicitly declares x and y.
    pass
```

### `global` and `nonlocal` variables

`global` and `nonlocal` declarations are always explicit declarations.
Note that these statements don't allow annotations.

```python
def f():
    global global_x     # Explicitly declares global_x.
    nonlocal nonlocal_x # Explicitly declares nonlocal_x.
    pass
```

### `with as` statements

`with as` statements explicitly declare a variable.
They must not shadow outer variables.
Note that these statements don't allow annotations.

```python
with open('general_kenobi.txt', 'w') as ofile:  # Explicitly declares `ofile`.
    ofile.write('Hello there!')
ofile.write("I have a bad feeling about this.") # Error: `ofile` is no longer declared.
```
```python
ofile : Any
with open('general_kenobi.txt', 'w') as ofile:  # Error: Must not shadow previously declared `ofile`.
    ofile.write('Hello there!')
```

### `for` statements

Note that `for` statements don't allow annotations.

`for` statements lazily declare a variable.
If the variable is not yet declared, the for statement declares it in its own scope.
If the variable is already declared, then this variable is re-used.
`for` variables cannot not shadow outer variables.

```python
for i in range(3,9): # Explicitly declares variable `i`.
    print(i)
print(i)             # Error: Variable `i` is no longer declared.
```
```python
i : int                 # Explicitly declares variable `i`.
for i in range(3,9):    # Uses previously declared variable `i`.
    print(i)
print(i)                # OK: Variable `i` is still declared.
```

List-comprehension `for`-statements also explicitly declare variables.
As lists are a nested scope, list-comprehensions do not re-use previously declared variables
and must not shadow outer variables.

```python
doubles : Any = [2 * n for n in range(50)]  # Explicitly declares `n` in the scope of this list-comprehension.
                                            # (Also explicitly declares `doubles`.)
```
```python
n : int
doubles : Any = [2 * n for n in range(50)]  # Error: list-comprehension must not shadow `n`.
```



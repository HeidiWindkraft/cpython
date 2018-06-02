"""
This is a test.
This code must result in a syntax error, because variable "variabel" is defined but never used.
"""

import strict_no_unused_vars;

def f(x):
    if (x > 1):
        variable = 42
    else:
        variabel = 24
    return variable

print(f(2))


# ...
"""
This test checks that usage is correctly propagated from
list constructors and nested functions.
"""

import strict_no_unused_vars;

def myfunc1(arg):
	myoffset = arg + 1
	return [ myoffset + i for i in range(0,1) ] # Here we use myoffset in a nested block, process this correctly.

def bury(x):
	""" Buries dead assignments."""
	pass

def myfunc2():
	myvar = 2
	dummy1 = 4
	dummy2 = 8
	del dummy1      # This counts as usage.
	bury(dummy2)    # This counts as usage.
	def myfunc3():
		global some_global_value
		some_global_value = 6           # This is not a dead assignment.
		x = myvar * myvar               # Here we use myvar in a nested scope.
		return myfunc1(x)
	return myfunc3

def goodsyntax_strict_main():
	global some_global_value
	return myfunc2()()[0] + some_global_value

result = (goodsyntax_strict_main() == 11)



#import strict_vars;

global_enter_count = 0;
global_exit_count = 0;

class EE:
	__slots__ = ('x', );
	step = 2;
	def __init__(self, y):
		self.x = y;
	def __enter__(self):
		global global_enter_count;
		global_enter_count += EE.step;
		return self.x;
	def __exit__(self, type, value, traceback):
		global global_exit_count;
		global_exit_count += self.step;

def my_for(a, b):
	i : int;
	s : int = 0;
	for i in range(0,a):
		s += i;
	for j in range(a,b):
		s += i;
	for i,j in enumerate(range(0,4)):
		s += i + j;
	return s;


def my_with1(a, b):
	x : int;
	with EE(a + b) as c:
		x = c;
	return x;
# `with`can't declare tuples as tuples have no __enter__ and __exit__ attributes.
#def my_with2(a, b):
#	x : int;
#	with (EE(a + b), EE(a - b)) as c, d:
#		x = c * d;
#	with (EE(a + b), EE(a - b)) as (c, d):
#		x = c * d;
#	return x;
def my_with3(a, b):
	x : int;
	with EE(a + b):
		pass;
	return x;

class Point:
	__slots__ = ('x', 'y');
	def __init__(self):
		self.x = 0;
		self.y = 0;

def my_except():
	try:
		pass;
	except ValueError as verr:
		print ("{0}".format(verr));

def my_listcomp1():
	doubles : Any = [2 * n for n in range(50)];

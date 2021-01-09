#!/usr/bin/env python3

import time
import timeit
import sys
import string
import random

def perf_counter_ns():
  if sys.version_info.major == 3 and sys.version_info.minor <= 6:
    return time.perf_counter() * 1000 * 1000 * 1000
  else:
    return time.perf_counter_ns()


def _timeit(*args, **kwargs):
  kwargs['timer'] = perf_counter_ns
  number = kwargs.get('number') or timeit.default_number
  total = timeit.timeit(*args, **kwargs)
  avg = total // number
  return avg


# https://pybenchmarks.org/u64q/program.php?test=fibonacci&lang=python3&id=3
def fib(n):
  def _powLF(n):
    if n == 1:
      return (1, 1)
    L, F = _powLF(n//2)
    L, F = (L**2 + 5*F**2) >> 1, L*F
    if n & 1:
      return ((L + 5*F) >> 1, (L + F) >> 1)
    else:
      return (L, F)
  if n & 1:
    return _powLF(n)[1]
  else:
    L, F = _powLF(n // 2)
    return L * F

def fib_recur(n):
  if n == 0:
    return 0
  elif n == 1:
    return 1
  else:
    return fib_recur(n-1) + fib_recur(n-2)


def report(*args):
  print("%-30s: %10s ns" % args)


print("Python: " + str(sys.version_info))
report('fib_recur(10):', _timeit(lambda: fib_recur(10), number=10000))
report('fib_recur(20):', _timeit(lambda: fib_recur(20), number=100))
report('fib(10):', _timeit(lambda: fib(10), number=10000))
report('fib(46):', _timeit(lambda: fib(46), number=10000))
report('fib(47):', _timeit(lambda: fib(47), number=10000))
report('fib(92):', _timeit(lambda: fib(92), number=10000))

report('instruction:', _timeit('pass', number=10000000))
report('empty for:', _timeit('for x in range(10000): pass', number=100))
report('while:', _timeit(
  """
i = 10000
while i>0:
  i=i-1
  """, number=10))
report('while2:', _timeit(
  """
i = 10000
while True:
  i=i-1
  if(i == 0):
    break
  """, number=10))



s1 = "This is a test"
s2 = " of the emergency broadcast system."
s3 = s1 + s2

report("string assign (s)", _timeit("s = s1", globals=globals(), number=1000000))
report("string assign (m)", _timeit("s = s3", globals=globals(), number=1000000))
report("string add (ss)", _timeit("s1 + s2", globals=globals(), number=1000000))
report("string add (sm)", _timeit("s1 + s3", globals=globals(), number=1000000))
report("string add (ms)", _timeit("s3 + s1", globals=globals(), number=1000000))
report("string add (mm)", _timeit("s3 + s3", globals=globals(), number=1000000))
report("string += (ss)", 0)
report("string += (sm)", 0)
report("string += (ms)", 0)
report("string += (mm)", 0)

report("allocate array", _timeit("[ 0 for x in range(100) ]"))
report("array creation (int)", _timeit("[ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ]"))
report("array creation (string)", _timeit('[ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" ]'))
a1 = [ 5, 1, 3, 4, 2 ]
a2 = [ 9, 8, 7, 6, 10 ]
a3 = [ 4, 1 ]
report("array assign", _timeit("a = a1", globals=globals()))
report("array addition", _timeit("a1 + a2", globals=globals()))
report("array subtraction", _timeit("[x for x in a1 if x not in a2]", globals=globals()))
report("array lookup", _timeit("a1[3]", globals=globals()))
report("array +=", 0)
report("array -=", 0)

report("allocate mapping", _timeit("{0: 0 for i in range(100)}"))
report("mapping creation (int)", _timeit("{1 : 2, 3 : 4, 5 : 6, 7 : 8}"))
report("mapping creation (string)", _timeit('{ "1" : "a", "2" : "b", "3" : "c", "4" : "d", "5" : "e"}'))
m1 = { "1" : "a", "2" : "b", "3" : "c", "4" : "d", "5" : "e" }
m2 = { 1 : "a", 2 : "b", 3 : "c", 4 : "d", 5 : "e" }
report("mapping assign", _timeit("m = m1", globals=globals()))
report("lookup string (exist)", _timeit('m1["3"]', globals=globals()))
report("lookup string (missing)", _timeit('m1.get("6")', globals=globals()))
report("lookup int (exist)", _timeit('m2[3]', globals=globals()))
report("lookup int (missing)", _timeit('m2.get(6)', globals=globals()))

from __future__ import print_function

import time

def fib(n):
  if n < 2: return n
  return fib(n - 1) + fib(n - 2)

start = time.perf_counter()
for i in range(0, 5):
  print(fib(28))
print("elapsed: " + str(time.perf_counter() - start))
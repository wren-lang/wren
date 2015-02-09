from __future__ import print_function

import time

map = {}

start = time.clock()

for i in range(1, 100001):
  map[i] = i

sum = 0
for i in range(1, 100001):
  sum = sum + map[i]
print(sum)

for i in range(1, 100001):
  del map[i]

print("elapsed: " + str(time.clock() - start))
from __future__ import print_function

import time

start = time.process_time()

map = {}

for i in range(1, 2000001):
  map[i] = i

sum = 0
for i in range(1, 2000001):
  sum = sum + map[i]
print(sum)

for i in range(1, 2000001):
  del map[i]

print("elapsed: " + str(time.process_time() - start))
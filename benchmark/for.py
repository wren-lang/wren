import time

start = time.clock()
list = []
for i in xrange(0, 1000000):
  list.append(i)

sum = 0
for i in list:
  sum += i
print(sum)
print("elapsed: " + str(time.clock() - start))
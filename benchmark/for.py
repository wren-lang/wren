import time

list = range(0, 2000000)

start = time.clock()
sum = 0
for i in list:
  sum += i
print(sum)
print("elapsed: " + str(time.clock() - start))
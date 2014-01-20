var list = []

var start = OS.clock
for (i in 0...1000000) list.add(i)

var sum = 0
for (i in list) sum = sum + i

IO.print(sum)
IO.print("elapsed: " + (OS.clock - start).toString)

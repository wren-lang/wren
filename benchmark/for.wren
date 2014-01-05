var list = []

for (i in 0...2000000) list.add(i)

var start = OS.clock
var sum = 0
for (i in list) {
  sum = sum + i
}
IO.print(sum)
IO.print("elapsed: " + (OS.clock - start).toString)

var start = IO.clock

var map = {}

for (i in 1..1000000) {
  map[i] = i
}

var sum = 0
for (i in 1..1000000) {
  sum = sum + map[i]
}
IO.print(sum)

for (i in 1..1000000) {
  map.remove(i)
}

IO.print("elapsed: ", IO.clock - start)

var start = System.clock

var map = {}

for (i in 1..2000000) {
  map[i] = i
}

var sum = 0
for (i in 1..2000000) {
  sum = sum + map[i]
}
System.print(sum)

for (i in 1..2000000) {
  map.remove(i)
}

System.print("elapsed: %(System.clock - start)")

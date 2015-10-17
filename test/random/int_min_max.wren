import "random" for Random

var random = Random.new(12345)

var counts = [0, 0, 0, 0, 0, 0, 0, 0]
for (i in 1..10000) {
  var n = random.int(3, 8)
  counts[n] = counts[n] + 1
}

for (i in 0..2) {
  if (counts[i] != 0) System.print("too low value")
}

for (i in 3..7) {
  if (counts[i] < 1900) System.print("too few")
  if (counts[i] > 2100) System.print("too many")
}

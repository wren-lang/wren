import "random" for Random

var random = Random.new(12345)

var counts = [0, 0, 0, 0, 0]
for (i in 1..10000) {
  var n = random.int(5)
  counts[n] = counts[n] + 1
}

for (count in counts) {
  if (count < 1900) System.print("too few")
  if (count > 2100) System.print("too many")
}

import "random" for Random

var random = Random.new(12345)

var below = 0
for (i in 1..100) {
  var n = random.float(2, 5)
  if (n < 2) System.print("too low")
  if (n >= 5) System.print("too high")
}

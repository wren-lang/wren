import "random" for Random

var random = Random.new(12345)

var below = 0
for (i in 1..1000) {
  var n = random.int()
  if (n < 2147483648) below = below + 1
}

// Should be roughly evenly distributed.
System.print(below > 450) // expect: true
System.print(below < 550) // expect: true

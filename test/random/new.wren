import "random" for Random

var random = Random.new()

var correct = 0
for (i in 1..100) {
  var n = random.float()
  if (n >= 0) correct = correct + 1
  if (n < 1) correct = correct + 1
}

System.print(correct) // expect: 200

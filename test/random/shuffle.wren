import "random" for Random

var random = Random.new(12345)

// Empty list.
var list = []
random.shuffle(list)
System.print(list) // expect: []

// One element.
list = [1]
random.shuffle(list)
System.print(list) // expect: [1]

// Given enough tries, should generate all permutations with roughly equal
// probability.
var histogram = {}
for (i in 1..5000) {
  var list = [1, 2, 3, 4]
  random.shuffle(list)

  var string = list.toString
  if (!histogram.containsKey(string)) histogram[string] = 0
  histogram[string] = histogram[string] + 1
}

System.print(histogram.count) // expect: 24
for (key in histogram.keys) {
  var error = (histogram[key] / (5000 / 24) - 1).abs
  if (error > 0.21) System.print("!!! %(error)")
}


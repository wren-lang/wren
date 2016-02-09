import "random" for Random

var random = Random.new(12345)

// Should choose all elements with roughly equal probability.
var list = ["a", "b", "c", "d"]
var histogram = {}
for (i in 1..5000) {
  var sample = random.sample(list, 3)
  var string = sample.toString
  if (!histogram.containsKey(string)) histogram[string] = 0
  histogram[string] = histogram[string] + 1
}

System.print(histogram.count) // expect: 24
for (key in histogram.keys) {
  var error = (histogram[key] / (5000 / 24) - 1).abs
  if (error > 0.2) System.print("!!! %(error)")
}

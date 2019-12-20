import "random" for Random

var random = Random.new(12345)

// Should choose all elements with roughly equal probability.
var list = ["a", "b", "c", "d", "e", "f"]
var histogram = {}
for (i in 1..5000) {
  var sample = random.sample(list, 4)
  // Represent the samples with an unordered set.
  var map = {}
  sample.each {|s| map[s] = 1 }
  var string = map.toString
  if (!histogram.containsKey(string)) histogram[string] = 0
  histogram[string] = histogram[string] + 1
}

System.print(histogram.count) // expect: 15
for (key in histogram.keys) {
  var error = (histogram[key] / (5000 / 15) - 1).abs
  if (error > 0.2) System.print("!!! %(error)")
}

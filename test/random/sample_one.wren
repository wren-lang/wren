import "random" for Random

var random = Random.new(12345)

// Single element list.
System.print(random.sample(["single"])) // expect: single

// Should choose all elements with roughly equal probability.
var list = ["a", "b", "c", "d", "e"]
var histogram = {"a": 0, "b": 0, "c": 0, "d": 0, "e": 0}
for (i in 1..1000) {
  var sample = random.sample(list)
  histogram[sample] = histogram[sample] + 1
}

System.print(histogram.count) // expect: 5
for (key in histogram.keys) {
  var error = (histogram[key] / (1000 / list.count) - 1).abs
  if (error > 0.2) System.print("!!! %(error)")
}

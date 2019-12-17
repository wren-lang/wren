import "random" for Random

var random = Random.new(12345)

// Single element list.
System.print(random.sample(["single"])) // expect: single

// Should choose all elements with roughly equal probability.
var testProbability = Fn.new { |list|
  var histogram = {}
  for (entry in list) histogram[entry] = 0

  for (i in 1..1000) {
    var sample = random.sample(list)
    histogram[sample] = histogram[sample] + 1
  }

  System.print(histogram.count)
  for (key in histogram.keys) {
    var error = (histogram[key] / (1000 / list.count) - 1).abs
    if (error > 0.2) System.print("!!! %(error)")
  }
}

// Depending on the size of the list a different algorithm is used, so test both branches.
testProbability.call(["a", "b", "c", "d", "e"]) // expect: 5
testProbability.call([0, 1, 2, 3, 4, 5, 6]) // expect: 7

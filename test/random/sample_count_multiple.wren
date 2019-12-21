import "random" for Random

var random = Random.new(12345)

// Should choose all elements with roughly equal probability.
var list = (0...10).toList
var binom = [1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1]

for (k in 0..10) {
  var count = binom[k]

  var histogram = {}
  for (i in 1..count * 100) {
    var sample = random.sample(list, k)
    // Create a bitmask to represent the unordered set.
    var bitmask = 0
    sample.each {|s| bitmask = bitmask | (1 << s) }
    if (!histogram.containsKey(bitmask)) histogram[bitmask] = 0
    histogram[bitmask] = histogram[bitmask] + 1
  }

  if (histogram.count != count) System.print("!!! %(count) %(histogram.count)")
  for (key in histogram.keys) {
    var error = (histogram[key] - 100).abs
    if (error > 50) System.print("!!! %(error)")
  }
}

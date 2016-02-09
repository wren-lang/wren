import "random" for Random

var random = Random.new(12345)

random.sample([1, 2, 3], 4) // expect runtime error: Not enough elements to sample.

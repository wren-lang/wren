import "random" for Random

var random = Random.new(12345)

random.sample([]) // expect runtime error: Not enough elements to sample.

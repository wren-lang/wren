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

// Given enough tries, should generate all permutations.
var hits = {}
for (i in 1..200) {
  var list = [1, 2, 3, 4]
  random.shuffle(list)
  hits[list.toString] = true
}

System.print(hits.count) // expect: 24

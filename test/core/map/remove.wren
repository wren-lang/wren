var map = {
  "one": 1,
  "two": 2,
  "three": 3
}

IO.print(map.count) // expect: 3
IO.print(map.remove("two")) // expect: 2
IO.print(map.count) // expect: 2
IO.print(map.remove("three")) // expect: 3
IO.print(map.count) // expect: 1

// Remove an already removed entry.
IO.print(map.remove("two")) // expect: null
IO.print(map.count) // expect: 1

IO.print(map.remove("one")) // expect: 1
IO.print(map.count) // expect: 0

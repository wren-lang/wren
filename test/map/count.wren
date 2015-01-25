var map = {}
IO.print(map.count) // expect: 0
map["one"] = "value"
IO.print(map.count) // expect: 1
map["two"] = "value"
IO.print(map.count) // expect: 2
map["three"] = "value"
IO.print(map.count) // expect: 3

// Adding existing key does not increase count.
map["two"] = "new value"
IO.print(map.count) // expect: 3

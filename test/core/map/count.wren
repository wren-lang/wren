var map = {}
System.print(map.count) // expect: 0
map["one"] = "value"
System.print(map.count) // expect: 1
map["two"] = "value"
System.print(map.count) // expect: 2
map["three"] = "value"
System.print(map.count) // expect: 3

// Adding existing key does not increase count.
map["two"] = "new value"
System.print(map.count) // expect: 3

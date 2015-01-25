var map = new Map

map[null] = "null value"
map[true] = "true value"
map[false] = "false value"
map[0] = "zero"
map[1.2] = "1 point 2"
map[List] = "list class"
map["null"] = "string value"
map[1..3] = "1 to 3"

IO.print(map[null]) // expect: null value
IO.print(map[true]) // expect: true value
IO.print(map[false]) // expect: false value
IO.print(map[0]) // expect: zero
IO.print(map[1.2]) // expect: 1 point 2
IO.print(map[List]) // expect: list class
IO.print(map["null"]) // expect: string value
IO.print(map[1..3]) // expect: 1 to 3

IO.print(map.count) // expect: 8

// Use the same keys (but sometimes different objects) to ensure keys have the
// right equality semantics.
map[null] = "new null value"
map[!false] = "new true value"
map[!true] = "new false value"
map[2 - 2] = "new zero"
map[1.2] = "new 1 point 2"
map[[].type] = "new list class"
map["nu" + "ll"] = "new string value"
map[(3 - 2)..(1 + 2)] = "new 1 to 3"

IO.print(map[null]) // expect: new null value
IO.print(map[true]) // expect: new true value
IO.print(map[false]) // expect: new false value
IO.print(map[0]) // expect: new zero
IO.print(map[1.2]) // expect: new 1 point 2
IO.print(map[List]) // expect: new list class
IO.print(map["null"]) // expect: new string value
IO.print(map[1..3]) // expect: new 1 to 3

IO.print(map.count) // expect: 8

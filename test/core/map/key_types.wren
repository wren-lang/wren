var fiber = Fiber.new {}

var map = {
  null: "null value",
  true: "true value",
  false: "false value",
  0: "zero",
  1.2: "1 point 2",
  List: "list class",
  "null": "string value",
  (1..3): "1 to 3"
}

System.print(map[null]) // expect: null value
System.print(map[true]) // expect: true value
System.print(map[false]) // expect: false value
System.print(map[0]) // expect: zero
System.print(map[1.2]) // expect: 1 point 2
System.print(map[List]) // expect: list class
System.print(map["null"]) // expect: string value
System.print(map[1..3]) // expect: 1 to 3

System.print(map.count) // expect: 8

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

System.print(map[null]) // expect: new null value
System.print(map[true]) // expect: new true value
System.print(map[false]) // expect: new false value
System.print(map[0]) // expect: new zero
System.print(map[1.2]) // expect: new 1 point 2
System.print(map[List]) // expect: new list class
System.print(map["null"]) // expect: new string value
System.print(map[1..3]) // expect: new 1 to 3

System.print(map.count) // expect: 8

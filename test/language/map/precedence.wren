var name = "value"

var map = {
  // Primary.
  name: name,
  1: true,

  // Call.
  name.count: name.count,
  name[0]: name[1],

  // Unary.
  -1: -2,
  ~3: !false,

  // Allow any expression for a value.
  "key": true ? 1 : 2
}

System.print(map[name])       // expect: value
System.print(map[1])          // expect: true
System.print(map[name.count]) // expect: 5
System.print(map[name[0]])    // expect: a
System.print(map[-1])         // expect: -2
System.print(map[~3])         // expect: true
System.print(map["key"])      // expect: 1

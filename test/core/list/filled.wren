var list = List.filled(3, "value")
System.print(list.count) // expect: 3
System.print(list) // expect: [value, value, value]

// Can create an empty list.
list = List.filled(0, "value")
System.print(list.count) // expect: 0
System.print(list) // expect: []

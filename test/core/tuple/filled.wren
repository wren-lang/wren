var tuple = Tuple.filled(3, "value")
System.print(tuple.count) // expect: 3
System.print(tuple) // expect: (value, value, value)

// Can create an empty tuple.
tuple = Tuple.filled(0, "value")
System.print(tuple.count) // expect: 0
System.print(tuple) // expect: ()

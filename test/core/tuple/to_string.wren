// Handle empty tuples.
System.print(Tuple.new().toString)                           // expect: ()

// Does not quote strings.
System.print(Tuple.new(1, "2", true).toString)               // expect: (1, 2, true)

// Nested tuples.
System.print(Tuple.new(1, Tuple.new(2, Tuple.new(3), 4), 5)) // expect: (1, (2, (3), 4), 5)

// TODO: Handle tuples that contain themselves.

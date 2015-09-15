// Inclusive.
var range = 1..3
System.print(range.iterate(null)) // expect: 1
System.print(range.iterate(1)) // expect: 2
System.print(range.iterate(2)) // expect: 3
System.print(range.iterate(3)) // expect: false
System.print(range.iterate(4)) // expect: false

// Exclusive
range = 1...3
System.print(range.iterate(null)) // expect: 1
System.print(range.iterate(1)) // expect: 2
System.print(range.iterate(2)) // expect: false

// Negative inclusive range.
range = 3..1
System.print(range.iterate(null)) // expect: 3
System.print(range.iterate(3)) // expect: 2
System.print(range.iterate(2)) // expect: 1
System.print(range.iterate(1)) // expect: false

// Negative exclusive range.
range = 3...1
System.print(range.iterate(null)) // expect: 3
System.print(range.iterate(3)) // expect: 2
System.print(range.iterate(2)) // expect: false

// Empty inclusive range.
range = 1..1
System.print(range.iterate(null)) // expect: 1
System.print(range.iterate(1)) // expect: false

// Empty exclusive range.
range = 1...1
System.print(range.iterate(null)) // expect: false

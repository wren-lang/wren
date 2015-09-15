// Ordered range.
System.print((2..5).max) // expect: 5
System.print((3..3).max) // expect: 3
System.print((0..3).max) // expect: 3
System.print((-5..3).max) // expect: 3
System.print((-5..-2).max) // expect: -2

// Backwards range.
System.print((5..2).max) // expect: 5
System.print((3..0).max) // expect: 3
System.print((3..-5).max) // expect: 3
System.print((-2..-5).max) // expect: -2

// Exclusive ordered range.
System.print((2...5).max) // expect: 5
System.print((3...3).max) // expect: 3
System.print((0...3).max) // expect: 3
System.print((-5...3).max) // expect: 3
System.print((-5...-2).max) // expect: -2

// Exclusive backwards range.
System.print((5...2).max) // expect: 5
System.print((3...0).max) // expect: 3
System.print((3...-5).max) // expect: 3
System.print((-2...-5).max) // expect: -2

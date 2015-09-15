// Ordered range.
System.print((2..5).min) // expect: 2
System.print((3..3).min) // expect: 3
System.print((0..3).min) // expect: 0
System.print((-5..3).min) // expect: -5
System.print((-5..-2).min) // expect: -5

// Backwards range.
System.print((5..2).min) // expect: 2
System.print((3..0).min) // expect: 0
System.print((3..-5).min) // expect: -5
System.print((-2..-5).min) // expect: -5

// Exclusive ordered range.
System.print((2...5).min) // expect: 2
System.print((3...3).min) // expect: 3
System.print((0...3).min) // expect: 0
System.print((-5...3).min) // expect: -5
System.print((-5...-2).min) // expect: -5

// Exclusive backwards range.
System.print((5...2).min) // expect: 2
System.print((3...0).min) // expect: 0
System.print((3...-5).min) // expect: -5
System.print((-2...-5).min) // expect: -5

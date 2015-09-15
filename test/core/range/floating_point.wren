// Ordered range.
System.print((2..5).from) // expect: 2
System.print((3..3).from) // expect: 3
System.print((0..3).from) // expect: 0
System.print((-5..3).from) // expect: -5
System.print((-5..-2).from) // expect: -5

// Backwards range.
System.print((5..2).from) // expect: 5
System.print((3..0).from) // expect: 3
System.print((3..-5).from) // expect: 3
System.print((-2..-5).from) // expect: -2

// Exclusive ordered range.
System.print((2...5).from) // expect: 2
System.print((3...3).from) // expect: 3
System.print((0...3).from) // expect: 0
System.print((-5...3).from) // expect: -5
System.print((-5...-2).from) // expect: -5

// Exclusive backwards range.
System.print((5...2).from) // expect: 5
System.print((3...0).from) // expect: 3
System.print((3...-5).from) // expect: 3
System.print((-2...-5).from) // expect: -2

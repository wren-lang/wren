// Ordered range.
System.print((2..5).to) // expect: 5
System.print((3..3).to) // expect: 3
System.print((0..3).to) // expect: 3
System.print((-5..3).to) // expect: 3
System.print((-5..-2).to) // expect: -2

// Backwards range.
System.print((5..2).to) // expect: 2
System.print((3..0).to) // expect: 0
System.print((3..-5).to) // expect: -5
System.print((-2..-5).to) // expect: -5

// Exclusive ordered range.
System.print((2...5).to) // expect: 5
System.print((3...3).to) // expect: 3
System.print((0...3).to) // expect: 3
System.print((-5...3).to) // expect: 3
System.print((-5...-2).to) // expect: -2

// Exclusive backwards range.
System.print((5...2).to) // expect: 2
System.print((3...0).to) // expect: 0
System.print((3...-5).to) // expect: -5
System.print((-2...-5).to) // expect: -5

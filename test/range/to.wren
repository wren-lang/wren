// Ordered range.
IO.print((2..5).to) // expect: 5
IO.print((3..3).to) // expect: 3
IO.print((0..3).to) // expect: 3
IO.print((-5..3).to) // expect: 3
IO.print((-5..-2).to) // expect: -2

// Backwards range.
IO.print((5..2).to) // expect: 2
IO.print((3..0).to) // expect: 0
IO.print((3..-5).to) // expect: -5
IO.print((-2..-5).to) // expect: -5

// Exclusive ordered range.
IO.print((2...5).to) // expect: 4
IO.print((3...3).to) // expect: 2
IO.print((0...3).to) // expect: 2
IO.print((-5...3).to) // expect: 2
IO.print((-5...-2).to) // expect: -3

// Exclusive backwards range.
IO.print((5...2).to) // expect: 1
IO.print((3...0).to) // expect: -1
IO.print((3...-5).to) // expect: -6
IO.print((-2...-5).to) // expect: -6

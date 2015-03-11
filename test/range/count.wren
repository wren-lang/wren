// Ordered range.
IO.print((2..5).count) // expect: 4
IO.print((3..3).count) // expect: 1
IO.print((0..3).count) // expect: 4
IO.print((-5..3).count) // expect: 9
IO.print((-5..-2).count) // expect: 4

// Backwards range.
IO.print((5..2).count) // expect: 4
IO.print((3..0).count) // expect: 4
IO.print((3..-5).count) // expect: 9
IO.print((-2..-5).count) // expect: 4

// Exclusive ordered range.
IO.print((2...5).count) // expect: 3
IO.print((3...3).count) // expect: 0
IO.print((0...3).count) // expect: 3
IO.print((-5...3).count) // expect: 8
IO.print((-5...-2).count) // expect: 3

// Exclusive backwards range.
IO.print((5...2).count) // expect: 3
IO.print((3...0).count) // expect: 3
IO.print((3...-5).count) // expect: 8
IO.print((-2...-5).count) // expect: 3

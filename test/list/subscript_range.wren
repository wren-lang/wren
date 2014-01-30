// Returns lists.
var list = ["a", "b", "c", "d", "e"]
IO.print(list[0..0]) // expect: [a]
IO.print(list[1...1]) // expect: []
IO.print(list[1..2]) // expect: [b, c]
IO.print(list[1...2]) // expect: [b]
IO.print(list[2..4]) // expect: [c, d, e]
IO.print(list[2...5]) // expect: [c, d, e]

// A backwards range reverses.
IO.print(list[3..1]) // expect: [d, c, b]
IO.print(list[3...1]) // expect: [d, c]
IO.print(list[3...3]) // expect: []

// Negative ranges index from the end.
IO.print(list[-5..-2]) // expect: [a, b, c, d]
IO.print(list[-5...-2]) // expect: [a, b, c]
IO.print(list[-3..-5]) // expect: [c, b, a]
IO.print(list[-3...-6]) // expect: [c, b, a]

// Half-negative ranges are treated like the negative value is fixed before
// walking the range.
IO.print(list[-5..3]) // expect: [a, b, c, d]
IO.print(list[-3...5]) // expect: [c, d, e]
IO.print(list[-2..1]) // expect: [d, c, b]
IO.print(list[-2...0]) // expect: [d, c, b]

IO.print(list[1..-2]) // expect: [b, c, d]
IO.print(list[2...-1]) // expect: [c, d]
IO.print(list[4..-5]) // expect: [e, d, c, b, a]
IO.print(list[3...-6]) // expect: [d, c, b, a]

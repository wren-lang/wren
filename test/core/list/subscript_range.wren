// Returns lists.
var list = ["a", "b", "c", "d", "e"]
System.print(list[0..0]) // expect: [a]
System.print(list[1...1]) // expect: []
System.print(list[1..2]) // expect: [b, c]
System.print(list[1...2]) // expect: [b]
System.print(list[2..4]) // expect: [c, d, e]
System.print(list[2...5]) // expect: [c, d, e]

// A backwards range reverses.
System.print(list[3..1]) // expect: [d, c, b]
System.print(list[3...1]) // expect: [d, c]
System.print(list[3...3]) // expect: []

// Negative ranges index from the end.
System.print(list[-5..-2]) // expect: [a, b, c, d]
System.print(list[-5...-2]) // expect: [a, b, c]
System.print(list[-3..-5]) // expect: [c, b, a]
System.print(list[-3...-6]) // expect: [c, b, a]

// Half-negative ranges are treated like the negative value is fixed before
// walking the range.
System.print(list[-5..3]) // expect: [a, b, c, d]
System.print(list[-3...5]) // expect: [c, d, e]
System.print(list[-2..1]) // expect: [d, c, b]
System.print(list[-2...0]) // expect: [d, c, b]

System.print(list[1..-2]) // expect: [b, c, d]
System.print(list[2...-1]) // expect: [c, d]
System.print(list[4..-5]) // expect: [e, d, c, b, a]
System.print(list[3...-6]) // expect: [d, c, b, a]

// An empty range at zero is allowed on an empty list.
System.print([][0...0]) // expect: []
System.print([][0..-1]) // expect: []

// An empty range at the end is allowed on a list.
System.print([1, 2, 3][3...3]) // expect: []
System.print([1, 2, 3][3..-1]) // expect: []

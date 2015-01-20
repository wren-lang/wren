var string = "abcde"
IO.print(string[0..0]) // expect: a
IO.print(string[1...1] == "") // expect: true
IO.print(string[1..2]) // expect: bc
IO.print(string[1...2]) // expect: b
IO.print(string[2..4]) // expect: cde
IO.print(string[2...5]) // expect: cde

// A backwards range reverses.
IO.print(string[3..1]) // expect: dcb
IO.print(string[3...1]) // expect: dc
IO.print(string[3...3] == "") // expect: true

// Negative ranges index from the end.
IO.print(string[-5..-2]) // expect: abcd
IO.print(string[-5...-2]) // expect: abc
IO.print(string[-3..-5]) // expect: cba
IO.print(string[-3...-6]) // expect: cba

// Half-negative ranges are treated like the negative value is fixed before
// walking the range.
IO.print(string[-5..3]) // expect: abcd
IO.print(string[-3...5]) // expect: cde
IO.print(string[-2..1]) // expect: dcb
IO.print(string[-2...0]) // expect: dcb

IO.print(string[1..-2]) // expect: bcd
IO.print(string[2...-1]) // expect: cd
IO.print(string[4..-5]) // expect: edcba
IO.print(string[3...-6]) // expect: dcba

// An empty range at zero is allowed on an empty string.
IO.print(""[0...0] == "") // expect: true
IO.print(""[0..-1] == "") // expect: true

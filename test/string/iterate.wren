var s = "abçd"
IO.print(s.iterate(null)) // expect: 0
IO.print(s.iterate(0)) // expect: 1
IO.print(s.iterate(1)) // expect: 2
// Skip 3 because that's the middle of the ç sequence.
IO.print(s.iterate(2)) // expect: 4
// Iterating from the middle of a UTF-8 sequence goes to the next one.
IO.print(s.iterate(3)) // expect: 4
IO.print(s.iterate(4)) // expect: false

// Out of bounds.
IO.print(s.iterate(123)) // expect: false
IO.print(s.iterate(-1)) // expect: false

// Nothing to iterate in an empty string.
IO.print("".iterate(null)) // expect: false

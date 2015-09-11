var codePoints = "abçd".codePoints
IO.print(codePoints.iterate(null)) // expect: 0
IO.print(codePoints.iterate(0)) // expect: 1
IO.print(codePoints.iterate(1)) // expect: 2
// Skip 3 because that's the middle of the ç sequence.
IO.print(codePoints.iterate(2)) // expect: 4
// Iterating from the middle of a UTF-8 sequence goes to the next one.
IO.print(codePoints.iterate(3)) // expect: 4
IO.print(codePoints.iterate(4)) // expect: false

// Out of bounds.
IO.print(codePoints.iterate(123)) // expect: false
IO.print(codePoints.iterate(-1)) // expect: false

// Nothing to iterate in an empty string.
IO.print("".codePoints.iterate(null)) // expect: false

// 8-bit clean.
IO.print("a\0b\0c".codePoints.iterate(null)) // expect: 0
IO.print("a\0b\0c".codePoints.iterate(0)) // expect: 1
IO.print("a\0b\0c".codePoints.iterate(1)) // expect: 2
IO.print("a\0b\0c".codePoints.iterate(2)) // expect: 3
IO.print("a\0b\0c".codePoints.iterate(3)) // expect: 4
IO.print("a\0b\0c".codePoints.iterate(4)) // expect: false

// Iterates over invalid UTF-8 one byte at a time.
IO.print("\xef\xf7".codePoints.iterate(null)) // expect: 0
IO.print("\xef\xf7".codePoints.iterate(0)) // expect: 1
IO.print("\xef\xf7".codePoints.iterate(1)) // expect: false

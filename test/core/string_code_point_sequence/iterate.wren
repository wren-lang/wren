var codePoints = "abçd".codePoints
System.print(codePoints.iterate(null)) // expect: 0
System.print(codePoints.iterate(0)) // expect: 1
System.print(codePoints.iterate(1)) // expect: 2
// Skip 3 because that's the middle of the ç sequence.
System.print(codePoints.iterate(2)) // expect: 4
// Iterating from the middle of a UTF-8 sequence goes to the next one.
System.print(codePoints.iterate(3)) // expect: 4
System.print(codePoints.iterate(4)) // expect: false

// Out of bounds.
System.print(codePoints.iterate(123)) // expect: false
System.print(codePoints.iterate(-1)) // expect: false

// Nothing to iterate in an empty string.
System.print("".codePoints.iterate(null)) // expect: false

// 8-bit clean.
System.print("a\0b\0c".codePoints.iterate(null)) // expect: 0
System.print("a\0b\0c".codePoints.iterate(0)) // expect: 1
System.print("a\0b\0c".codePoints.iterate(1)) // expect: 2
System.print("a\0b\0c".codePoints.iterate(2)) // expect: 3
System.print("a\0b\0c".codePoints.iterate(3)) // expect: 4
System.print("a\0b\0c".codePoints.iterate(4)) // expect: false

// Iterates over invalid UTF-8 one byte at a time.
System.print("\xef\xf7".codePoints.iterate(null)) // expect: 0
System.print("\xef\xf7".codePoints.iterate(0)) // expect: 1
System.print("\xef\xf7".codePoints.iterate(1)) // expect: false

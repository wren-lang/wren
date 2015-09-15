var s = "abçd"
System.print(s.iterate(null)) // expect: 0
System.print(s.iterate(0)) // expect: 1
System.print(s.iterate(1)) // expect: 2
// Skip 3 because that's the middle of the ç sequence.
System.print(s.iterate(2)) // expect: 4
// Iterating from the middle of a UTF-8 sequence goes to the next one.
System.print(s.iterate(3)) // expect: 4
System.print(s.iterate(4)) // expect: false

// Out of bounds.
System.print(s.iterate(123)) // expect: false
System.print(s.iterate(-1)) // expect: false

// Nothing to iterate in an empty string.
System.print("".iterate(null)) // expect: false

// 8-bit clean.
System.print("a\0b\0c".iterate(null)) // expect: 0
System.print("a\0b\0c".iterate(0)) // expect: 1
System.print("a\0b\0c".iterate(1)) // expect: 2
System.print("a\0b\0c".iterate(2)) // expect: 3
System.print("a\0b\0c".iterate(3)) // expect: 4
System.print("a\0b\0c".iterate(4)) // expect: false

// Iterates over invalid UTF-8 one byte at a time.
System.print("\xef\xf7".iterate(null)) // expect: 0
System.print("\xef\xf7".iterate(0)) // expect: 1
System.print("\xef\xf7".iterate(1)) // expect: false

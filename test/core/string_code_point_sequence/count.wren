System.print("".codePoints.count)   // expect: 0
System.print("a string".codePoints.count) // expect: 8

// 8-bit clean.
System.print("\0".codePoints.count)  // expect: 1
System.print("a\0b".codePoints.count)  // expect: 3
System.print("\0c".codePoints.count)  // expect: 2
System.print(("a\0b" + "\0c").codePoints.count)  // expect: 5

// Treats a UTF-8 sequence as a single item.
//
// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
System.print("søméஃthîng".codePoints.count) // expect: 10

// Counts invalid UTF-8 one byte at a time.
System.print("\xefok\xf7".codePoints.count) // expect: 4

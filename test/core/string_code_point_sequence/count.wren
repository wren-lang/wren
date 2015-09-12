IO.print("".codePoints.count)   // expect: 0
IO.print("a string".codePoints.count) // expect: 8

// 8-bit clean.
IO.print("\0".codePoints.count)  // expect: 1
IO.print("a\0b".codePoints.count)  // expect: 3
IO.print("\0c".codePoints.count)  // expect: 2
IO.print(("a\0b" + "\0c").codePoints.count)  // expect: 5

// Treats a UTF-8 sequence as a single item.
//
// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
IO.print("søméஃthîng".codePoints.count) // expect: 10

// Counts invalid UTF-8 one byte at a time.
IO.print("\xefok\xf7".codePoints.count) // expect: 4

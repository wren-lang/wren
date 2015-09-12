IO.print("".count)   // expect: 0
IO.print("a string".count) // expect: 8

// 8-bit clean.
IO.print("\0".count)  // expect: 1
IO.print("a\0b".count)  // expect: 3
IO.print("\0c".count)  // expect: 2
IO.print(("a\0b" + "\0c").count)  // expect: 5

// Treats a UTF-8 sequence as a single item.
//
// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
IO.print("søméஃthîng".count) // expect: 10

// Counts invalid UTF-8 one byte at a time.
IO.print("\xefok\xf7".count) // expect: 4

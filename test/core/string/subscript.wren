// Returns characters (as strings).
System.print("abcd"[0]) // expect: a
System.print("abcd"[1]) // expect: b
System.print("abcd"[2]) // expect: c
System.print("abcd"[3]) // expect: d

// Allows indexing backwards from the end.
System.print("abcd"[-4]) // expect: a
System.print("abcd"[-3]) // expect: b
System.print("abcd"[-2]) // expect: c
System.print("abcd"[-1]) // expect: d

// Regression: Make sure the string's internal buffer size is correct.
System.print("abcd"[1] == "b") // expect: true

// Indexes by byte, not code point.
//
// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
System.print("søméஃthîng"[0]) // expect: s
System.print("søméஃthîng"[1]) // expect: ø
System.print("søméஃthîng"[3]) // expect: m
System.print("søméஃthîng"[6]) // expect: ஃ
System.print("søméஃthîng"[10]) // expect: h
System.print("søméஃthîng"[-1]) // expect: g
System.print("søméஃthîng"[-2]) // expect: n
System.print("søméஃthîng"[-4]) // expect: î

// If the subscript is in the middle of a UTF-8 sequence, return the raw byte.
System.print("søméஃthîng"[2] == "\xb8") // expect: true
System.print("søméஃthîng"[7] == "\xae") // expect: true
System.print("søméஃthîng"[8] == "\x83") // expect: true
System.print("søméஃ"[-1] == "\x83") // expect: true
System.print("søméஃ"[-2] == "\xae") // expect: true

// 8-bit clean.
System.print("a\0b\0c"[0] == "a") // expect: true
System.print("a\0b\0c"[1] == "\0") // expect: true
System.print("a\0b\0c"[2] == "b") // expect: true
System.print("a\0b\0c"[3] == "\0") // expect: true
System.print("a\0b\0c"[4] == "c") // expect: true

// Returns single byte strings for invalid UTF-8 sequences.
System.print("\xef\xf7"[0] == "\xef") // expect: true
System.print("\xef\xf7"[1] == "\xf7") // expect: true

// Returns characters (as strings).
IO.print("abcd"[0]) // expect: a
IO.print("abcd"[1]) // expect: b
IO.print("abcd"[2]) // expect: c
IO.print("abcd"[3]) // expect: d

// Allows indexing backwards from the end.
IO.print("abcd"[-4]) // expect: a
IO.print("abcd"[-3]) // expect: b
IO.print("abcd"[-2]) // expect: c
IO.print("abcd"[-1]) // expect: d

// Regression: Make sure the string's internal buffer size is correct.
IO.print("abcd"[1] == "b") // expect: true

// Indexes by byte, not code point.
//
// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
IO.print("søméஃthîng"[0]) // expect: s
IO.print("søméஃthîng"[1]) // expect: ø
IO.print("søméஃthîng"[3]) // expect: m
IO.print("søméஃthîng"[6]) // expect: ஃ
IO.print("søméஃthîng"[10]) // expect: h
IO.print("søméஃthîng"[-1]) // expect: g
IO.print("søméஃthîng"[-2]) // expect: n
IO.print("søméஃthîng"[-4]) // expect: î

// If the subscript is in the middle of a UTF-8 sequence, yield an empty string.
IO.print("søméஃthîng"[2] == "") // expect: true
IO.print("søméஃthîng"[7] == "") // expect: true
IO.print("søméஃthîng"[8] == "") // expect: true
IO.print("søméஃ"[-1] == "") // expect: true
IO.print("søméஃ"[-2] == "") // expect: true

// 8-bit clean.
IO.print("a\0b\0c"[0] == "a") // expect: true
IO.print("a\0b\0c"[1] == "\0") // expect: true
IO.print("a\0b\0c"[2] == "b") // expect: true
IO.print("a\0b\0c"[3] == "\0") // expect: true
IO.print("a\0b\0c"[4] == "c") // expect: true

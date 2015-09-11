// Simple.
IO.print("".bytes.count) // expect: 0
IO.print("123".bytes.count) // expect: 3

// UTF-8.
// Bytes:
//        123456789
// Chars: sø mé ஃ
IO.print("søméஃ".bytes.count) // expect: 9

// Null bytes.
IO.print("\0\0\0".bytes.count) // expect: 3

// Invalid UTF-8.
IO.print("\xef\x00".bytes.count) // expect: 2

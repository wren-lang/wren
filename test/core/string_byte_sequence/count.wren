// Simple.
System.print("".bytes.count) // expect: 0
System.print("123".bytes.count) // expect: 3

// UTF-8.
// Bytes:
//        123456789
// Chars: sø mé ஃ
System.print("søméஃ".bytes.count) // expect: 9

// Null bytes.
System.print("\0\0\0".bytes.count) // expect: 3

// Invalid UTF-8.
System.print("\xef\xf7".bytes.count) // expect: 2

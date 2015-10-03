// One byte UTF-8 Sequences.
System.print("\u0041")     // expect: A
System.print("\u007e")     // expect: ~

// Two byte sequences.
System.print("\u00b6")     // expect: ¶
System.print("\u00de")     // expect: Þ

// Three byte sequences.
System.print("\u0950")     // expect: ॐ
System.print("\u0b83")     // expect: ஃ

// Capitalized hex.
System.print("\u00B6")     // expect: ¶
System.print("\u00DE")     // expect: Þ

// Big escapes:
var smile = "\U0001F603"
var byteSmile = "\xf0\x9f\x98\x83"
System.print(byteSmile == smile) // expect: true

System.print("<\U0001F64A>")       // expect: <🙊>
System.print("<\U0001F680>")       // expect: <🚀>
System.print("<\U00010318>")       // expect: <𐌘>

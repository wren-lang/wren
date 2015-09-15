var s = "abçd"
System.print(s.iteratorValue(0)) // expect: a
System.print(s.iteratorValue(1)) // expect: b
System.print(s.iteratorValue(2)) // expect: ç
// Iterator value in middle of UTF sequence is the unencoded byte.
System.print(s.iteratorValue(3) == "\xa7") // expect: true
System.print(s.iteratorValue(4)) // expect: d

// 8-bit clean.
var t = "a\0b\0c"
System.print(t.iteratorValue(0) == "a") // expect: true
System.print(t.iteratorValue(1) == "\0") // expect: true
System.print(t.iteratorValue(2) == "b") // expect: true
System.print(t.iteratorValue(3) == "\0") // expect: true
System.print(t.iteratorValue(4) == "c") // expect: true

// Returns single byte strings for invalid UTF-8 sequences.
System.print("\xef\xf7".iteratorValue(0) == "\xef") // expect: true
System.print("\xef\xf7".iteratorValue(1) == "\xf7") // expect: true

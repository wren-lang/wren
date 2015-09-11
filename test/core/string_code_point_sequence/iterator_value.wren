// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
var codePoints = "søméஃthîng".codePoints

IO.print(codePoints.iteratorValue(0)) // expect: 115
IO.print(codePoints.iteratorValue(1)) // expect: 248
IO.print(codePoints.iteratorValue(2)) // expect: -1
IO.print(codePoints.iteratorValue(3)) // expect: 109
IO.print(codePoints.iteratorValue(4)) // expect: 233
IO.print(codePoints.iteratorValue(5)) // expect: -1
IO.print(codePoints.iteratorValue(6)) // expect: 2947
IO.print(codePoints.iteratorValue(7)) // expect: -1
IO.print(codePoints.iteratorValue(8)) // expect: -1
IO.print(codePoints.iteratorValue(9)) // expect: 116
IO.print(codePoints.iteratorValue(10)) // expect: 104
IO.print(codePoints.iteratorValue(11)) // expect: 238
IO.print(codePoints.iteratorValue(12)) // expect: -1
IO.print(codePoints.iteratorValue(13)) // expect: 110
IO.print(codePoints.iteratorValue(14)) // expect: 103

IO.print(codePoints.iteratorValue(-15)) // expect: 115
IO.print(codePoints.iteratorValue(-14)) // expect: 248
IO.print(codePoints.iteratorValue(-13)) // expect: -1
IO.print(codePoints.iteratorValue(-12)) // expect: 109
IO.print(codePoints.iteratorValue(-11)) // expect: 233
IO.print(codePoints.iteratorValue(-10)) // expect: -1
IO.print(codePoints.iteratorValue(-9)) // expect: 2947
IO.print(codePoints.iteratorValue(-8)) // expect: -1
IO.print(codePoints.iteratorValue(-7)) // expect: -1
IO.print(codePoints.iteratorValue(-6)) // expect: 116
IO.print(codePoints.iteratorValue(-5)) // expect: 104
IO.print(codePoints.iteratorValue(-4)) // expect: 238
IO.print(codePoints.iteratorValue(-3)) // expect: -1
IO.print(codePoints.iteratorValue(-2)) // expect: 110
IO.print(codePoints.iteratorValue(-1)) // expect: 103

IO.print("\0".codePoints.iteratorValue(0)) // expect: 0

// Returns -1 for invalid UTF-8 sequences.
IO.print("\xef\xf7".codePoints.iteratorValue(0)) // expect: -1
IO.print("\xef\xf7".codePoints.iteratorValue(1)) // expect: -1

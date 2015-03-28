// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
var s = "søméஃthîng"

IO.print(s.codePointAt(0)) // expect: 115
IO.print(s.codePointAt(1)) // expect: 248
IO.print(s.codePointAt(2)) // expect: -1
IO.print(s.codePointAt(3)) // expect: 109
IO.print(s.codePointAt(4)) // expect: 233
IO.print(s.codePointAt(5)) // expect: -1
IO.print(s.codePointAt(6)) // expect: 2947
IO.print(s.codePointAt(7)) // expect: -1
IO.print(s.codePointAt(8)) // expect: -1
IO.print(s.codePointAt(9)) // expect: 116
IO.print(s.codePointAt(10)) // expect: 104
IO.print(s.codePointAt(11)) // expect: 238
IO.print(s.codePointAt(12)) // expect: -1
IO.print(s.codePointAt(13)) // expect: 110
IO.print(s.codePointAt(14)) // expect: 103

IO.print(s.codePointAt(-15)) // expect: 115
IO.print(s.codePointAt(-14)) // expect: 248
IO.print(s.codePointAt(-13)) // expect: -1
IO.print(s.codePointAt(-12)) // expect: 109
IO.print(s.codePointAt(-11)) // expect: 233
IO.print(s.codePointAt(-10)) // expect: -1
IO.print(s.codePointAt(-9)) // expect: 2947
IO.print(s.codePointAt(-8)) // expect: -1
IO.print(s.codePointAt(-7)) // expect: -1
IO.print(s.codePointAt(-6)) // expect: 116
IO.print(s.codePointAt(-5)) // expect: 104
IO.print(s.codePointAt(-4)) // expect: 238
IO.print(s.codePointAt(-3)) // expect: -1
IO.print(s.codePointAt(-2)) // expect: 110
IO.print(s.codePointAt(-1)) // expect: 103

IO.print("\0".codePointAt(0)) // expect: 0

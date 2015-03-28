// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
var s = "søméஃthîng"

IO.print(s.byteAt(0)) // expect: 115
IO.print(s.byteAt(1)) // expect: 195
IO.print(s.byteAt(2)) // expect: 184
IO.print(s.byteAt(3)) // expect: 109
IO.print(s.byteAt(4)) // expect: 195
IO.print(s.byteAt(5)) // expect: 169
IO.print(s.byteAt(6)) // expect: 224
IO.print(s.byteAt(7)) // expect: 174
IO.print(s.byteAt(8)) // expect: 131
IO.print(s.byteAt(9)) // expect: 116
IO.print(s.byteAt(10)) // expect: 104
IO.print(s.byteAt(11)) // expect: 195
IO.print(s.byteAt(12)) // expect: 174
IO.print(s.byteAt(13)) // expect: 110
IO.print(s.byteAt(14)) // expect: 103

IO.print(s.byteAt(-15)) // expect: 115
IO.print(s.byteAt(-14)) // expect: 195
IO.print(s.byteAt(-13)) // expect: 184
IO.print(s.byteAt(-12)) // expect: 109
IO.print(s.byteAt(-11)) // expect: 195
IO.print(s.byteAt(-10)) // expect: 169
IO.print(s.byteAt(-9)) // expect: 224
IO.print(s.byteAt(-8)) // expect: 174
IO.print(s.byteAt(-7)) // expect: 131
IO.print(s.byteAt(-6)) // expect: 116
IO.print(s.byteAt(-5)) // expect: 104
IO.print(s.byteAt(-4)) // expect: 195
IO.print(s.byteAt(-3)) // expect: 174
IO.print(s.byteAt(-2)) // expect: 110
IO.print(s.byteAt(-1)) // expect: 103

IO.print("\0".byteAt(0)) // expect: 0

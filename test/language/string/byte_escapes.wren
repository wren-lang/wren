var s = "\x00\x12\x34\x56\x78\xab\xCD\xfFf"

IO.print(s.byteAt(0)) // expect: 0
IO.print(s.byteAt(1)) // expect: 18
IO.print(s.byteAt(2)) // expect: 52
IO.print(s.byteAt(3)) // expect: 86
IO.print(s.byteAt(4)) // expect: 120
IO.print(s.byteAt(5)) // expect: 171
IO.print(s.byteAt(6)) // expect: 205
IO.print(s.byteAt(7)) // expect: 255
// "f".
IO.print(s.byteAt(8)) // expect: 102

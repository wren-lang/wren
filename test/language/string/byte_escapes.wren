var bytes = "\x00\x12\x34\x56\x78\xab\xCD\xfFf".bytes

IO.print(bytes[0]) // expect: 0
IO.print(bytes[1]) // expect: 18
IO.print(bytes[2]) // expect: 52
IO.print(bytes[3]) // expect: 86
IO.print(bytes[4]) // expect: 120
IO.print(bytes[5]) // expect: 171
IO.print(bytes[6]) // expect: 205
IO.print(bytes[7]) // expect: 255
// "f".
IO.print(bytes[8]) // expect: 102

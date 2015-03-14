var x = 0xFF

IO.print(x) // expect: 255
IO.print(x + 1) // expect: 256
IO.print(x == 255) // expect: true
IO.print(0x09 is Num) // expect: true
IO.print(x is Num) // expect: true
IO.print(-0xFF) // expect: -255
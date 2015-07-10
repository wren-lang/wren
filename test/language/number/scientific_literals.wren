var x = 2.55e2

IO.print(x) // expect: 255
IO.print(x + 1) // expect: 256
IO.print(x == 255) // expect: true
IO.print(2.55e-2 is Num) // expect: true
IO.print(x is Num) // expect: true
IO.print(-2.55e2) // expect: -255
IO.print(-25500e-2) // expect: -255

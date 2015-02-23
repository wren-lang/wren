IO.print(2 | 1 << 1) // expect: 2
IO.print(1 << 1 | 2) // expect: 2
IO.print(2 & 1 << 1) // expect: 2
IO.print(1 << 1 & 2) // expect: 2
IO.print(2 ^ 1 << 1) // expect: 0
IO.print(1 << 1 ^ 2) // expect: 0

IO.print(1 & 1 | 2) // expect: 3
IO.print(2 | 1 & 1) // expect: 3
IO.print(1 & 1 ^ 2) // expect: 3
IO.print(2 ^ 1 & 1) // expect: 3
IO.print(1 ^ 1 | 1) // expect: 1
IO.print(1 | 1 ^ 1) // expect: 1

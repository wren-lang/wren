IO.print("abs")
IO.print(Math.abs(123))      // expect: 123
IO.print(Math.abs(-123))   // expect: 123
IO.print(Math.abs(0))        // expect: 0
IO.print(Math.abs(-0))     // expect: 0
IO.print(Math.abs(-0.12))  // expect: 0.12
IO.print(Math.abs(12.34))    // expect: 12.34

IO.print(Math.abs(1.0)) // expect: 1
IO.print(Math.abs(-1.0)) // expect: 1

IO.print("ceil")
IO.print(Math.ceil(2.3)) // expect: 3
IO.print(Math.ceil(3.8)) // expect: 4
IO.print(Math.ceil(-2.3)) // expect: -2
IO.print(Math.ceil(-3.8)) // expect: -3

IO.print("floor")
IO.print(Math.floor(2.3)) // expect: 2
IO.print(Math.floor(3.8)) // expect: 3
IO.print(Math.floor(-2.3)) // expect: -3
IO.print(Math.floor(-3.8)) // expect: -4

IO.print("int")
IO.print(Math.int(8)) // expect: 8
IO.print(Math.int(12.34)) // expect: 12
IO.print(Math.int(-8)) // expect: -8
IO.print(Math.int(-12.34)) // expect: -12

IO.print("frac")
IO.print(Math.frac(8)) // expect: 0
IO.print(Math.frac(12.34)) // expect: 0.34
IO.print(Math.frac(-8)) // expect: -0
IO.print(Math.frac(-12.34)) // expect: -0.34

IO.print("srand/rand")
Math.srand
for (i in 1..10) {
  IO.print(Math.floor(Math.rand * 100))
}

IO.print(8 / 2)         // expect: 4
IO.print(12.34 / -0.4)  // expect: -30.85

// Divide by zero.
IO.print(3 / 0)         // expect: inf
IO.print(-3 / 0)        // expect: -inf
IO.print(0 / 0)         // expect: nan
IO.print(-0 / 0)        // expect: nan
IO.print(3 / -0)        // expect: -inf
IO.print(-3 / -0)       // expect: inf
IO.print(0 / -0)        // expect: nan
IO.print(-0 / -0)       // expect: nan

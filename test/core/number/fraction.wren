IO.print(123.fraction)      // expect: 0
IO.print((-123).fraction)   // expect: -0
IO.print(0.fraction)        // expect: 0
IO.print((-0).fraction)     // expect: -0
IO.print(0.123.fraction)    // expect: 0.123
IO.print(12.3.fraction)     // expect: 0.3
IO.print((-0.123).fraction) // expect: -0.123
IO.print((-12.3).fraction)  // expect: -0.3

// Using 32-bit representation, a longer mantissa will lead to
// approximation.
IO.print((1.23456789012345).fraction)  // expect: 0.23456789012345
IO.print((-1.23456789012345).fraction)  // expect: -0.23456789012345

IO.print((0.000000000000000000000000000000000000000001).fraction)  // expect: 1e-42
IO.print((-0.000000000000000000000000000000000000000001).fraction)  // expect: -1e-42

IO.print((1.000000000000000000000000000000000000000001).fraction)  // expect: 0
IO.print((-1.000000000000000000000000000000000000000001).fraction)  // expect: -0

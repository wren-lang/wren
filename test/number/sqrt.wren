IO.print(4.sqrt)        // expect: 2
IO.print(1000000.sqrt)  // expect: 1000
IO.print(1.sqrt)        // expect: 1
IO.print(-0.sqrt)       // expect: -0
IO.print(0.sqrt)        // expect: 0
IO.print(2.sqrt)        // expect: 1.4142135623731

IO.print(-4.sqrt.isNan) // expect: true

// TODO: Tests for sin and cos.

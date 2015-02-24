IO.print(123)     // expect: 123
IO.print(987654)  // expect: 987654
IO.print(0)       // expect: 0
IO.print(-0)      // expect: -0

IO.print(123.456) // expect: 123.456
IO.print(-0.001)  // expect: -0.001

IO.print(0xabcdef)      // expect: 11259375
IO.print(0xABCDEF)      // expect: 11259375
IO.print(0x1234567890)  // expect: 78187493520
IO.print(0x0)           // expect: 0
IO.print(-0x0)          // expect: -0

// TODO: Scientific notation?
// TODO: Literals at and beyond numeric limits.

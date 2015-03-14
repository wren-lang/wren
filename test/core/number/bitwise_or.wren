IO.print(0 | 0) // expect: 0
IO.print(2863311530 | 1431655765) // expect: 4294967295
IO.print(3435973836 | 1717986918) // expect: 4008636142

// Max u32 value.
IO.print(4294967295 | 4294967295) // expect: 4294967295

// Past max u32 value.
IO.print(4294967296 | 4294967296) // expect: 0

// TODO: Negative numbers.
// TODO: Floating-point numbers.

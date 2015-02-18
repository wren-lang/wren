IO.print(0 << 0) // expect: 0
IO.print(1 << 0) // expect: 1
IO.print(0 << 1) // expect: 0
IO.print(1 << 1) // expect: 2
IO.print(2863311530 << 1) // expect: 1431655764
IO.print(4042322160 << 1) // expect: 3789677024

// Max u32 value.
IO.print(4294967295 << 1) // expect: 4294967294

// Past max u32 value.
IO.print(4294967296 << 1) // expect: 0

// TODO: Negative numbers.
// TODO: Floating-point numbers.

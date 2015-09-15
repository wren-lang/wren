System.print(0 ^ 0) // expect: 0
System.print(1 ^ 1) // expect: 0
System.print(0 ^ 1) // expect: 1
System.print(1 ^ 0) // expect: 1
System.print(2863311530 ^ 1431655765) // expect: 4294967295
System.print(4042322160 ^ 1010580540) // expect: 3435973836

// Max u32 value.
System.print(4294967295 ^ 4294967295) // expect: 0

// Past max u32 value.
System.print(4294967296 ^ 4294967296) // expect: 0

// TODO: Negative numbers.
// TODO: Floating-point numbers.

System.print(0 & 0) // expect: 0
System.print(2863311530 & 1431655765) // expect: 0
System.print(4042322160 & 1010580540) // expect: 808464432

// Max u32 value.
System.print(4294967295 & 4294967295) // expect: 4294967295

// Past max u32 value.
System.print(4294967296 & 4294967296) // expect: 0

// TODO: Negative numbers.
// TODO: Floating-point numbers.

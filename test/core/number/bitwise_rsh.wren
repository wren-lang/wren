System.print(0 >> 0) // expect: 0
System.print(1 >> 0) // expect: 1
System.print(0 >> 1) // expect: 0
System.print(1 >> 1) // expect: 0
System.print(2863311530 >> 1) // expect: 1431655765
System.print(4042322160 >> 1) // expect: 2021161080

// Max u32 value.
System.print(4294967295 >> 1) // expect: 2147483647

// Past max u32 value.
System.print(4294967296 >> 1) // expect: 0

// TODO: Negative numbers.
// TODO: Floating-point numbers.

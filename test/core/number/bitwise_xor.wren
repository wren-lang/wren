System.print(0 ^ 0) // expect: 0
System.print(1 ^ 1) // expect: 0
System.print(0 ^ 1) // expect: 1
System.print(1 ^ 0) // expect: 1
System.print(0xaaaaaaaa ^ 0x55555555) // expect: 4294967295
System.print(0xf0f0f0f0 ^ 0x3c3c3c3c) // expect: 3435973836

// Max u32 value.
System.print(0xffffffff ^ 0xffffffff) // expect: 0

// TODO: Negative numbers.
// TODO: Floating-point numbers.
// TODO: Numbers that don't fit in u32.

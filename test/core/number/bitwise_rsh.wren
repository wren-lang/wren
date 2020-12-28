System.print(0 >> 0) // expect: 0
System.print(1 >> 0) // expect: 1
System.print(0 >> 1) // expect: 0
System.print(1 >> 1) // expect: 0
System.print(0xaaaaaaaa >> 1) // expect: 1431655765
System.print(0xf0f0f0f0 >> 1) // expect: 2021161080

// Max u32 value.
System.print(0xffffffff >> 1) // expect: 2147483647
System.print(0xffffffff >> 32) // expect: 0

// TODO: Negative numbers.
// TODO: Floating-point numbers.
// TODO: Numbers that don't fit in u32.

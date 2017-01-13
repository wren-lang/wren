System.print(0 | 0) // expect: 0
System.print(0xaaaaaaaa | 0x55555555) // expect: 4294967295
System.print(0xcccccccc | 0x66666666) // expect: 4008636142

// Max u32 value.
System.print(0xffffffff | 0xffffffff) // expect: 4294967295

// TODO: Negative numbers.
// TODO: Floating-point numbers.
// TODO: Numbers that don't fit in u32.

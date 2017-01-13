System.print(~0) // expect: 4294967295
System.print(~1) // expect: 4294967294
System.print(~23) // expect: 4294967272

// Max u32 value.
System.print(~0xffffffff) // expect: 0

// Negative numbers.
System.print(~-1) // expect: 0
System.print(~-123) // expect: 122
System.print(~-0xfffffffe) // expect: 4294967293
System.print(~-0xffffffff) // expect: 4294967294

// Floating point values.
System.print(~1.23) // expect: 4294967294
System.print(~0.00123) // expect: 4294967295
System.print(~345.67) // expect: 4294966950
System.print(~-12.34) // expect: 11

// TODO: Numbers that don't fit in u32.

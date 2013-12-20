io.write(~0) // expect: 4294967295
io.write(~1) // expect: 4294967294
io.write(~23) // expect: 4294967272

// Max u32 value.
io.write(~4294967295) // expect: 0

// Past max u32 value.
io.write(~4294967296) // expect: 4294967295

// Negative numbers.
io.write(~-1) // expect: 0
io.write(~-123) // expect: 122
io.write(~-4294967294) // expect: 4294967293
io.write(~-4294967295) // expect: 4294967294
io.write(~-4294967296) // expect: 4294967295

// Floating point values.
io.write(~1.23) // expect: 4294967294
io.write(~0.00123) // expect: 4294967295
io.write(~345.67) // expect: 4294966950
io.write(~-12.34) // expect: 11

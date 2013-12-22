IO.write(~0) // expect: 4294967295
IO.write(~1) // expect: 4294967294
IO.write(~23) // expect: 4294967272

// Max u32 value.
IO.write(~4294967295) // expect: 0

// Past max u32 value.
IO.write(~4294967296) // expect: 4294967295

// Negative numbers.
IO.write(~-1) // expect: 0
IO.write(~-123) // expect: 122
IO.write(~-4294967294) // expect: 4294967293
IO.write(~-4294967295) // expect: 4294967294
IO.write(~-4294967296) // expect: 4294967295

// Floating point values.
IO.write(~1.23) // expect: 4294967294
IO.write(~0.00123) // expect: 4294967295
IO.write(~345.67) // expect: 4294966950
IO.write(~-12.34) // expect: 11

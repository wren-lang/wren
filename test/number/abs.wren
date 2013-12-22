IO.write(123.abs)    // expect: 123
IO.write(-123.abs)   // expect: 123
IO.write(0.abs)      // expect: 0
IO.write(-0.abs)     // expect: 0
IO.write(-0.12.abs)  // expect: 0.12
IO.write(12.34.abs)  // expect: 12.34

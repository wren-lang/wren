io.write(123.abs)    // expect: 123
io.write(-123.abs)   // expect: 123
io.write(0.abs)      // expect: 0
io.write(-0.abs)     // expect: 0
io.write(-0.12.abs)  // expect: 0.12
io.write(12.34.abs)  // expect: 12.34

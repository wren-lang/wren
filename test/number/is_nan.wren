IO.print(1.isNan)      // expect: false
IO.print((0/0).isNan)  // expect: true

// Infinity is not NaN.
IO.print((1/0).isNan)  // expect: false

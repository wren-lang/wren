io.write(5 % 3) // expect: 2
io.write(10 % 5) // expect: 0
io.write(-4 % 3) // expect: -1
io.write(4 % -3) // expect: 1
io.write(-4 % -3) // expect: -1
io.write(-4.2 % 3.1) // expect: -1.1
io.write(4.2 % -3.1) // expect: 1.1
io.write(-4.2 % -3.1) // expect: -1.1

// Left associative.
io.write(13 % 7 % 4) // expect: 2

// TODO: Unsupported RHS types.
// TODO: Error on mod by zero.

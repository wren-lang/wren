IO.write(5 % 3) // expect: 2
IO.write(10 % 5) // expect: 0
IO.write(-4 % 3) // expect: -1
IO.write(4 % -3) // expect: 1
IO.write(-4 % -3) // expect: -1
IO.write(-4.2 % 3.1) // expect: -1.1
IO.write(4.2 % -3.1) // expect: 1.1
IO.write(-4.2 % -3.1) // expect: -1.1

// Left associative.
IO.write(13 % 7 % 4) // expect: 2

// TODO: Unsupported RHS types.
// TODO: Error on mod by zero.

var inclusive = 2..5
IO.write(inclusive is Range) // expect: true
IO.write(inclusive.min) // expect: 2
IO.write(inclusive.max) // expect: 5

var exclusive = 2...5
IO.write(exclusive is Range) // expect: true
IO.write(exclusive.min) // expect: 2
IO.write(exclusive.max) // expect: 4

// TODO: Non-number RHS.
// TODO: Non-integer RHS.
// TODO: Range iteration.
// TODO: Empty or negative ranges.

var inclusive = 2..5
IO.print(inclusive is Range) // expect: true
IO.print(inclusive.min) // expect: 2
IO.print(inclusive.max) // expect: 5

var exclusive = 2...5
IO.print(exclusive is Range) // expect: true
IO.print(exclusive.min) // expect: 2
IO.print(exclusive.max) // expect: 4

// TODO: Non-number RHS.
// TODO: Non-integer RHS.
// TODO: Range iteration.
// TODO: Empty or negative ranges.

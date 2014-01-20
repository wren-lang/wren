var range = 1..3
IO.print(range.iteratorValue(1)) // expect: 1
IO.print(range.iteratorValue(2)) // expect: 2
IO.print(range.iteratorValue(3)) // expect: 3

// Doesn't bother to bounds check.
IO.print(range.iteratorValue(-2)) // expect: -2
IO.print(range.iteratorValue(5)) // expect: 5

// Or type check.
IO.print(range.iteratorValue("s")) // expect: s

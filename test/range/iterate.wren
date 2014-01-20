var range = 1..3
IO.print(range.iterate(null)) // expect: 1
IO.print(range.iterate(1)) // expect: 2
IO.print(range.iterate(2)) // expect: 3
IO.print(range.iterate(3)) // expect: false
IO.print(range.iterate(4)) // expect: false

// TODO: Negative and empty ranges.

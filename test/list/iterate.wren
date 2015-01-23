var a = ["one", "two", "three", "four"]
IO.print(a.iterate(null)) // expect: 0
IO.print(a.iterate(0)) // expect: 1
IO.print(a.iterate(1)) // expect: 2
IO.print(a.iterate(2)) // expect: 3
IO.print(a.iterate(3)) // expect: false

// Out of bounds.
IO.print(a.iterate(123)) // expect: false
IO.print(a.iterate(-1)) // expect: false

// Nothing to iterate in an empty list.
IO.print([].iterate(null)) // expect: false

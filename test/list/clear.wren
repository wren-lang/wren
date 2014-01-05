var a = [1, 2, 3]
a.clear
IO.print(a)       // expect: []
IO.print(a.count) // expect: 0

// Returns null.
IO.print([1, 2].clear) // expect: null

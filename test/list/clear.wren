var a = [1, 2, 3]
a.clear
IO.write(a)       // expect: []
IO.write(a.count) // expect: 0

// Returns null.
IO.write([1, 2].clear) // expect: null

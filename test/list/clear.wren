var a = [1, 2, 3]
a.clear
io.write(a)       // expect: []
io.write(a.count) // expect: 0

// Returns null.
io.write([1, 2].clear) // expect: null

// Returns elements.
var list = ["a", "b", "c", "d"]
io.write(list[0]) // expect: a
io.write(list[1]) // expect: b
io.write(list[2]) // expect: c
io.write(list[3]) // expect: d

// Allows indexing backwards from the end.
io.write(list[-4]) // expect: a
io.write(list[-3]) // expect: b
io.write(list[-2]) // expect: c
io.write(list[-1]) // expect: d

// Handle out of bounds.
// TODO(bob): Should halt the fiber or raise an error somehow.
io.write(list[4])  // expect: null
io.write(list[-5]) // expect: null

// Handle wrong argument type.
// TODO(bob): Should halt the fiber or raise an error somehow.
io.write(list[true]) // expect: null

// Handle non-integer index.
// TODO(bob): Should halt the fiber or raise an error somehow.
io.write(list[1.5]) // expect: null

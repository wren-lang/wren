// Returns elements.
var list = ["a", "b", "c", "d"]
IO.write(list[0]) // expect: a
IO.write(list[1]) // expect: b
IO.write(list[2]) // expect: c
IO.write(list[3]) // expect: d

// Allows indexing backwards from the end.
IO.write(list[-4]) // expect: a
IO.write(list[-3]) // expect: b
IO.write(list[-2]) // expect: c
IO.write(list[-1]) // expect: d

// Handle out of bounds.
// TODO: Should halt the fiber or raise an error somehow.
IO.write(list[4])  // expect: null
IO.write(list[-5]) // expect: null

// Handle wrong argument type.
// TODO: Should halt the fiber or raise an error somehow.
IO.write(list[true]) // expect: null

// Handle non-integer index.
// TODO: Should halt the fiber or raise an error somehow.
IO.write(list[1.5]) // expect: null

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

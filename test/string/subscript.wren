// Returns characters (as strings).
IO.write("abcd"[0]) // expect: a
IO.write("abcd"[1]) // expect: b
IO.write("abcd"[2]) // expect: c
IO.write("abcd"[3]) // expect: d

// Allows indexing backwards from the end.
IO.write("abcd"[-4]) // expect: a
IO.write("abcd"[-3]) // expect: b
IO.write("abcd"[-2]) // expect: c
IO.write("abcd"[-1]) // expect: d

// Handle out of bounds.
// TODO: Should halt the fiber or raise an error somehow.
IO.write("abcd"[4])  // expect: null
IO.write("abcd"[-5]) // expect: null

// Handle wrong argument type.
// TODO: Should halt the fiber or raise an error somehow.
IO.write("abcd"[true]) // expect: null

// Handle non-integer index.
// TODO: Should halt the fiber or raise an error somehow.
IO.write("abcd"[1.5]) // expect: null

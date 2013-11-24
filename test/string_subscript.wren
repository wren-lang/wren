// Returns characters (as strings).
io.write("abcd"[0]) // expect: a
io.write("abcd"[1]) // expect: b
io.write("abcd"[2]) // expect: c
io.write("abcd"[3]) // expect: d

// Allows indexing backwards from the end.
io.write("abcd"[-4]) // expect: a
io.write("abcd"[-3]) // expect: b
io.write("abcd"[-2]) // expect: c
io.write("abcd"[-1]) // expect: d

// Handle out of bounds.
// TODO(bob): Should halt the fiber or raise an error somehow.
io.write("abcd"[4])  // expect: null
io.write("abcd"[-5]) // expect: null

// Handle wrong argument type.
// TODO(bob): Should halt the fiber or raise an error somehow.
io.write("abcd"[true]) // expect: null

// Handle non-integer index.
// TODO(bob): Should halt the fiber or raise an error somehow.
io.write("abcd"[1.5]) // expect: null

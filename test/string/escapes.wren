// Escape characters.
io.write("\"")     // expect: "
io.write("\\")     // expect: \
io.write("(\n)")   // expect: (
                   // expect: )

// TODO: Non-printing escapes like \t.
// TODO: Unicode escape sequences.

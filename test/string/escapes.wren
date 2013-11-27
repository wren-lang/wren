// Escape characters.
io.write("\"")     // expect: "
io.write("\\")     // expect: \
io.write("(\n)")   // expect: (
                   // expect: )

// TODO(bob): Non-printing escapes like \t.
// TODO(bob): Unicode escape sequences.

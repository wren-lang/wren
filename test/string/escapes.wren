// Escape characters.
IO.write("\"")     // expect: "
IO.write("\\")     // expect: \
IO.write("(\n)")   // expect: (
                   // expect: )

// TODO: Non-printing escapes like \t.
// TODO: Unicode escape sequences.

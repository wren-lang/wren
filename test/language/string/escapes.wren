// Escape characters.
IO.print("\"")     // expect: "
IO.print("\\")     // expect: \
IO.print("(\n)")   // expect: (
                   // expect: )

// TODO: Non-printing escapes like \t.

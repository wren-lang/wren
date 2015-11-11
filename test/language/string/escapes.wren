// Escape characters.
System.print("\"")     // expect: "
System.print("\\")     // expect: \
System.print("(\n)")   // expect: (
                       // expect: )
System.print("\%")     // expect: %

// TODO: Non-printing escapes like \t.

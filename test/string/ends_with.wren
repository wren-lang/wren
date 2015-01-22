IO.print("abcd".endsWith("cd")) // expect: true
IO.print("abcd".endsWith("abcde")) // expect: false
IO.print("abcd".endsWith("abcd")) // expect: true
IO.print("abcd".endsWith("f")) // expect: false
IO.print("abcd".endsWith("")) // expect: true

// Non-ASCII.
IO.print("søméthîng".endsWith("thîng"))  // expect: true
IO.print("søméthîng".endsWith("thing")) // expect: false

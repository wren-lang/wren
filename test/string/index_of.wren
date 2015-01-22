IO.print("abcd".indexOf("cd")) // expect: 2
IO.print("abcd".indexOf("a")) // expect: 0
IO.print("abcd".indexOf("abcde")) // expect: -1
IO.print("abab".indexOf("ab")) // expect: 0

// Non-ASCII. Note that it returns byte indices, not code points.
IO.print("søméஃthîng".indexOf("e")) // expect: -1
IO.print("søméஃthîng".indexOf("m")) // expect: 3
IO.print("søméஃthîng".indexOf("thî")) // expect: 9

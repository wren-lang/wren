IO.print("abcd".startsWith("cd")) // expect: false
IO.print("abcd".startsWith("a")) // expect: true
IO.print("abcd".startsWith("abcd")) // expect: true
IO.print("abcd".startsWith("abcde")) // expect: false
IO.print("abcd".startsWith("")) // expect: true

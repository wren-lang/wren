var list = [
  "a",
  "b",
]

IO.print(list[0]) // expect: a
IO.print(list[1]) // expect: b

// Invalid syntax
IO.print(new Fiber { Meta.eval("[,]")    }.try()) // expect: Could not compile source code.
IO.print(new Fiber { Meta.eval("[1,,]")  }.try()) // expect: Could not compile source code.
IO.print(new Fiber { Meta.eval("[1,,2]") }.try()) // expect: Could not compile source code.

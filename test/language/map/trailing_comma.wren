var map = {
  "a": 1,
  "b": 2,
}

IO.print(map["a"]) // expect: 1
IO.print(map["b"]) // expect: 2

// Invalid syntax
// Parentheses are necessary to have these interpret as maps and not as blocks.
IO.print(new Fiber { Meta.eval("({,})")        }.try()) // expect: Could not compile source code.
IO.print(new Fiber { Meta.eval("({1:1,,})")    }.try()) // expect: Could not compile source code.
IO.print(new Fiber { Meta.eval("({1:1,,2:2})") }.try()) // expect: Could not compile source code.

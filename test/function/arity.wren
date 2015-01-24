IO.print(new Fn {}.arity) // expect: 0
IO.print(new Fn {|a| a}.arity) // expect: 1
IO.print(new Fn {|a, b| a}.arity) // expect: 2
IO.print(new Fn {|a, b, c| a}.arity) // expect: 3
IO.print(new Fn {|a, b, c, d| a}.arity) // expect: 4

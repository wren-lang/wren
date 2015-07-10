IO.print(Fn.new {}.arity) // expect: 0
IO.print(Fn.new {|a| a}.arity) // expect: 1
IO.print(Fn.new {|a, b| a}.arity) // expect: 2
IO.print(Fn.new {|a, b, c| a}.arity) // expect: 3
IO.print(Fn.new {|a, b, c, d| a}.arity) // expect: 4

System.print(Fn.new {}.arity) // expect: 0
System.print(Fn.new {|a| a}.arity) // expect: 1
System.print(Fn.new {|a, b| a}.arity) // expect: 2
System.print(Fn.new {|a, b, c| a}.arity) // expect: 3
System.print(Fn.new {|a, b, c, d| a}.arity) // expect: 4

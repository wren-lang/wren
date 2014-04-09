var f2 = new Fn {|a, b| IO.print(a, b) }
f2.call("a") // expect runtime error: Function expects more arguments.

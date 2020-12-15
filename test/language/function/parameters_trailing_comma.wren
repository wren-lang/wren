var f = Fn.new {|a, b,| System.print("%(a) %(b)") }

f.call(1, 2) // expect: 1 2

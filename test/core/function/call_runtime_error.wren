var f1 = Fn.new {|a, b| a + b } // expect runtime error: Bool does not implement '+(_)'.
f1.call(true, false)


IO.print([1].reduce {|a, b| 42 }) // expect: 1
IO.print([].reduce(1) {|a, b| 42 }) // expect: 1

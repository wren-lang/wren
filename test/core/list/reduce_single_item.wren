System.print([1].reduce {|a, b| 42 }) // expect: 1
System.print([].reduce(1) {|a, b| 42 }) // expect: 1

var range = 1..10

IO.print(range.reduce {|a, b| a + b }) // expect: 55
IO.print(range.reduce(100) {|a, b| a < b ? a : b }) // expect: 1

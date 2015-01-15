var a = [1, 4, 2, 1, 5]
var max = new Fn {|a, b| a > b ? a : b }
var sum = new Fn {|a, b| a + b }

IO.print(a.reduce(max)) // expect: 5
IO.print(a.reduce(10, max)) // expect: 10

IO.print(a.reduce(sum)) // expect: 13
IO.print(a.reduce(-1, sum)) // expect: 12

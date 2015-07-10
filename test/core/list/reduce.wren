var a = [1, 4, 2, 1, 5]
var b = ["W", "o", "r", "l", "d"]
var max = Fn.new {|a, b| a > b ? a : b }
var sum = Fn.new {|a, b| a + b }

IO.print(a.reduce(max)) // expect: 5
IO.print(a.reduce(10, max)) // expect: 10

IO.print(a.reduce(sum)) // expect: 13
IO.print(a.reduce(-1, sum)) // expect: 12

// sum also concatenates strings
IO.print(b.reduce("Hello ", sum)) // expect: Hello World
IO.print(b.reduce(sum)) // expect: World

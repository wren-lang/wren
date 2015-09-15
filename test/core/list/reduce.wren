var a = [1, 4, 2, 1, 5]
var b = ["W", "o", "r", "l", "d"]
var max = Fn.new {|a, b| a > b ? a : b }
var sum = Fn.new {|a, b| a + b }

System.print(a.reduce(max)) // expect: 5
System.print(a.reduce(10, max)) // expect: 10

System.print(a.reduce(sum)) // expect: 13
System.print(a.reduce(-1, sum)) // expect: 12

// sum also concatenates strings
System.print(b.reduce("Hello ", sum)) // expect: Hello World
System.print(b.reduce(sum)) // expect: World

var a = [1, 4, 2, 1, 5]
var b = ["W", "o", "r", "l", "d"]

System.print(a.reduce {|a, b| a > b ? a : b }) // expect: 5
System.print(a.reduce(10) {|a, b| a > b ? a : b }) // expect: 10

System.print(a.reduce {|a, b| a + b }) // expect: 13
System.print(a.reduce(-1) {|a, b| a + b }) // expect: 12

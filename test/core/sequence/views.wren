
var a = [0, 1, 2, 3]
var id = View.new()
System.print(Algorithm.join(a | id, ", ")) // expect: 0, 1, 2, 3

var t = TransformView.new {|value| value * value }

System.print(Algorithm.join(a | t, ", ")) // expect: 0, 1, 4, 9
System.print(Algorithm.join(a | t | t, ", ")) // expect: 0, 1, 16, 81

var evens = FilterView.new {|value| value % 2 == 0}
var odds = FilterView.new {|value| value % 2 == 1}
var evens_odds = evens | odds

System.print(Algorithm.join(a | evens, ", ")) // expect: 0, 2
System.print(Algorithm.join(a | odds, ", ")) // expect: 1, 3
System.print(Algorithm.join(a | evens | odds, ", ")) // expect: 
System.print(Algorithm.join(a | evens_odds, ", ")) // expect: 

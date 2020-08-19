var l = [10, 7, 8, 9, 1, 5]
l.sort{|a, b| a < b }
System.print(l) // expect: [1, 5, 7, 8, 9, 10]
l.sort{|a, b| a > b }
System.print(l) // expect: [10, 9, 8, 7, 5, 1]

[10, 7, 8, 9, 1, 5].sort(3) // expect runtime error: Comp must be a function.
System.print([].sum) // expect: 0
System.print([1, 2].sum) // expect: 3
System.print([3.44, 6.56, 0.01].sum) // expect: 10.01

System.print((1..5).sum) // expect: 15
System.print((1...5).sum) // expect: 10

System.print(["not a numeric", 1, 2].sum) // expect runtime error: Can't sum non-numeric values

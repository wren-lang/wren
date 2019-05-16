System.print([1, 2].sum) // expect: 3
System.print([3.44, 6.56, 0.01].sum) // expect: 10.01

System.print((1..5).sum) // expect: 15
System.print((1...5).sum) // expect: 10

System.print(["Hello", ", ", "world!"].sum) // expect: Hello, world!

System.print([[], []].sum) // expect: []

System.print([].sum) // expect runtime error: Can't sum an empty sequence.

var a = [1]
a.add(2)
System.print(a) // expect: [1, 2]
a.add(3)
System.print(a) // expect: [1, 2, 3]

// Returns added element.
System.print(a.add(4)) // expect: 4

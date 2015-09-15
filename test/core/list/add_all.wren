var a = [1]
a.addAll([2, 3])
System.print(a) // expect: [1, 2, 3]
a.addAll([])
System.print(a) // expect: [1, 2, 3]
a.addAll(4..6)
System.print(a) // expect: [1, 2, 3, 4, 5, 6]

// Returns argument.
var range = 7..9
System.print(a.addAll(range) == range) // expect: true

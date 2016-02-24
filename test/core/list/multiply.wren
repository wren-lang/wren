System.print([1, 2, 3] * 0) // expect: []
System.print([1, 2, 3] * 1) // expect: [1, 2, 3]
System.print([1, 2, 3] * 4) // expect: [1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3]

// Doesn't modify original list.
var a = [1, 2, 3]
a * 5
System.print(a) // expect: [1, 2, 3]

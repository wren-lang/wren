System.print([1, 2, 3] + [4, 5, 6]) // expect: [1, 2, 3, 4, 5, 6]
System.print([1, 2, 3] + (4..6)) // expect: [1, 2, 3, 4, 5, 6]
System.print([1, 2, 3] + "abc") // expect: [1, 2, 3, a, b, c]
System.print([] + []) // expect: []
System.print([1, 2] + []) // expect: [1, 2]
System.print([] + [3, 4]) // expect: [3, 4]

// Doesn't modify original list.
var a = [1, 2, 3]
a * 5
System.print(a) // expect: [1, 2, 3]

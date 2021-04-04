var a = [1, 2, 3]
a.remove(2)
System.print(a) // expect: [1, 3]

var b = [1, 2, 3]
b.remove(1)
System.print(b) // expect: [2, 3]

var c = [1, 2, 3]
c.remove(3)
System.print(c) // expect: [1, 2]

// Return the removed value.
System.print([3, 4, 5].remove(4)) // expect: 4
System.print([3, 4, 5].remove(5)) // expect: 5

// Return null when not found
System.print([1, 2, 3].remove(8)) // expect: null

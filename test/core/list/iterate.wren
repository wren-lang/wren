var a = ["one", "two", "three", "four"]
System.print(a.iterate(null)) // expect: 0
System.print(a.iterate(0)) // expect: 1
System.print(a.iterate(1)) // expect: 2
System.print(a.iterate(2)) // expect: 3
System.print(a.iterate(3)) // expect: false

// Out of bounds.
System.print(a.iterate(123)) // expect: false
System.print(a.iterate(-1)) // expect: false

// Nothing to iterate in an empty list.
System.print([].iterate(null)) // expect: false

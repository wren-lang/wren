var range = 1..3
System.print(range.iteratorValue(1)) // expect: 1
System.print(range.iteratorValue(2)) // expect: 2
System.print(range.iteratorValue(3)) // expect: 3

// Doesn't bother to bounds check.
System.print(range.iteratorValue(-2)) // expect: -2
System.print(range.iteratorValue(5)) // expect: 5

// Or type check.
System.print(range.iteratorValue("s")) // expect: s

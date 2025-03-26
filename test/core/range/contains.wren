// Ordered range.
System.print((2..5).contains(1)) // expect: false
System.print((2..5).contains(2)) // expect: true
System.print((2..5).contains(5)) // expect: true
System.print((2..5).contains(6)) // expect: false

// Negative ordered range
System.print((-2..-5).contains(-1)) // expect: false
System.print((-2..-5).contains(-2)) // expect: true
System.print((-2..-5).contains(-5)) // expect: true
System.print((-2..-5).contains(-6)) // expect: false

// Backwards range.
System.print((5..2).contains(1)) // expect: false
System.print((5..2).contains(2)) // expect: true
System.print((5..2).contains(5)) // expect: true
System.print((5..2).contains(6)) // expect: false

// Negative backwards range.
System.print((-5..-2).contains(-1)) // expect: false
System.print((-5..-2).contains(-2)) // expect: true
System.print((-5..-2).contains(-5)) // expect: true
System.print((-5..-2).contains(-6)) // expect: false

// Exclusive ordered range.
System.print((2...5).contains(1)) // expect: false
System.print((2...5).contains(2)) // expect: true
System.print((2...5).contains(5)) // expect: false
System.print((2...5).contains(6)) // expect: false

// Negative exclusive ordered range.
System.print((-2...-5).contains(-1)) // expect: false
System.print((-2...-5).contains(-2)) // expect: true
System.print((-2...-5).contains(-5)) // expect: false
System.print((-2...-5).contains(-6)) // expect: false

// Exclusive backwards range.
System.print((5...2).contains(1)) // expect: false
System.print((5...2).contains(2)) // expect: false
System.print((5...2).contains(5)) // expect: true
System.print((5...2).contains(6)) // expect: false

// Negative exclusive backwards range.
System.print((-5...-2).contains(-1)) // expect: false
System.print((-5...-2).contains(-2)) // expect: false
System.print((-5...-2).contains(-5)) // expect: true
System.print((-5...-2).contains(-6)) // expect: false

// Exlusive empty range.
System.print((2...2).contains(2)) // expect: false
System.print((-2...-2).contains(-2)) // expect: false

// Only values that are produced by the iterable should be
// matchable by contains
System.print((2..5).contains(3.5)) // expect: false
System.print((-2..-5).contains(-3.5)) // expect: false

System.print(Num.fromString("123") == 123) // expect: true
System.print(Num.fromString("-123") == -123) // expect: true
System.print(Num.fromString("-0") == -0) // expect: true
System.print(Num.fromString("12.34") == 12.34) // expect: true
System.print(Num.fromString("-0.0001") == -0.0001) // expect: true
System.print(Num.fromString(" 12 ") == 12) // expect: true

// Test some non-number literals and ensure they return null.
System.print(Num.fromString("test1") == null) // expect: true
System.print(Num.fromString("") == null) // expect: true
System.print(Num.fromString("prefix1.2") == null) // expect: true
System.print(Num.fromString("1.2suffix") == null) // expect: true

// Whitespace-only: strtod performs no conversion (end == start); without an
// early return the trailing-whitespace loop consumes the string and we
// incorrectly return 0 instead of null.
System.print(Num.fromString("   ") == null) // expect: true

// TODO: Parse hex and scientific numbers.

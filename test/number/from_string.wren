IO.print(Num.fromString("123") == 123) // expect: true
IO.print(Num.fromString("-123") == -123) // expect: true
IO.print(Num.fromString("-0") == -0) // expect: true
IO.print(Num.fromString("12.34") == 12.34) // expect: true
IO.print(Num.fromString("-0.0001") == -0.0001) // expect: true
IO.print(Num.fromString(" 12 ") == 12) // expect: true
IO.print(Num.fromString("1.2prefix") == 1.2) // expect: true

// Test some non-number literals and ensure they return null.
IO.print(Num.fromString("test1") == null) // expect: true
IO.print(Num.fromString("") == null) // expect: true
IO.print(Num.fromString("prefix1.2") == null) // expect: true


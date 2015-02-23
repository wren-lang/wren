IO.print(Num.fromString("123") == 123) // expect: true
IO.print(Num.fromString("-123") == -123) // expect: true
IO.print(Num.fromString("-0") == -0) // expect: true
IO.print(Num.fromString("12.34") == 12.34) // expect: true
IO.print(Num.fromString("-0.0001") == -0.0001) // expect: true

// Test a non-number literal and ensure it returns null.
IO.print(Num.fromString("test1") == null) // expect: true

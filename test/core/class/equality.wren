System.print(Num == Num)  // expect: true
System.print(Num == Bool) // expect: false

// Not equal to other types.
System.print(Num == 123)  // expect: false
System.print(Num == true) // expect: false

System.print(Num != Num)  // expect: false
System.print(Num != Bool) // expect: true

// Not equal to other types.
System.print(Num != 123)  // expect: true
System.print(Num != true) // expect: true

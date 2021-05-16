System.print("abc def ghi".toLowerCase) // expect: abc def ghi
System.print("ABC DEF GHI".toLowerCase) // expect: abc def ghi
System.print("AbC dEf GhI".toLowerCase) // expect: abc def ghi

// Empty string
System.print("".toLowerCase) // expect: ""

System.print("ABC DEF GHI".toLowerCase is String) // expect: true
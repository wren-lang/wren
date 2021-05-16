System.print("abc def ghi".toLowerCase() == "abc def ghi") // expect: true
System.print("ABC DEF GHI".toLowerCase() == "abc def ghi") // expect: true
System.print("AbC dEf GhI".toLowerCase() == "abc def ghi") // expect: true

// Empty string
System.print("".toLowerCase() == "") // expect: true

// Type
System.print("ABC DEF GHI".toLowerCase() is String) // expect: true

// UTF-8
System.print("søMéஃTHînGsøméஃ".toLowerCase() == "søméஃthîngsøméஃ") // expect: true

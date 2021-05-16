System.print("abc def ghi".lowerCase == "abc def ghi") // expect: true
System.print("ABC DEF GHI".lowerCase == "abc def ghi") // expect: true
System.print("AbC dEf GhI".lowerCase == "abc def ghi") // expect: true

// Empty string
System.print("".lowerCase == "") // expect: true

// Type
System.print("ABC DEF GHI".lowerCase is String) // expect: true

// UTF-8
System.print("søMéஃTHînGsøméஃ".lowerCase == "søméஃthîngsøméஃ") // expect: true

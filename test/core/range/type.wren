var range = 2..5

System.print(range is Range)      // expect: true
System.print(range is Sequence)   // expect: true
System.print(range is Object)     // expect: true
System.print(range is String)     // expect: false
System.print(range.type == Range) // expect: true

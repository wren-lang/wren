System.print(fn () { 0 } is Fn)      // expect: true
System.print(fn () { 0 } is Object)  // expect: true
System.print(fn () { 0 } is String)  // expect: false
System.print(fn () { 0 }.type == Fn) // expect: true

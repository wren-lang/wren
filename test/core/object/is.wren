System.print(Num is Class) // expect: true
System.print(true is Bool) // expect: true
System.print(Fn.new { 1 } is Fn) // expect: true
System.print(123 is Num) // expect: true
System.print(null is Null) // expect: true
System.print("s" is String) // expect: true

System.print(Num is Bool) // expect: false
System.print(null is Class) // expect: false
System.print(true is Fn) // expect: false
System.print(Fn.new { 1 } is Num) // expect: false
System.print("s" is Null) // expect: false
System.print(123 is String) // expect: false

// Everything extends Object.
System.print(Num is Object) // expect: true
System.print(null is Object) // expect: true
System.print(true is Object) // expect: true
System.print(Fn.new { 1 } is Object) // expect: true
System.print("s" is Object) // expect: true
System.print(123 is Object) // expect: true

// Classes extend Class.
System.print(Num is Class) // expect: true
System.print(null is Class) // expect: false
System.print(true is Class) // expect: false
System.print(Fn.new { 1 } is Class) // expect: false
System.print("s" is Class) // expect: false
System.print(123 is Class) // expect: false

// Ignore newline after "is".
System.print(123 is
  Num) // expect: true
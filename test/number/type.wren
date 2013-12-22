IO.write(123 is Num)      // expect: true
IO.write(123 is Object)   // expect: true
IO.write(123 is String)   // expect: false
IO.write(123.type == Num) // expect: true

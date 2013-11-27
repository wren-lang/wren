io.write(123 is Num)      // expect: true
io.write(123 is Object)   // expect: true
io.write(123 is String)   // expect: false
io.write(123.type == Num) // expect: true

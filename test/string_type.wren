io.write("s" is String)      // expect: true
io.write("s" is Object)      // expect: true
io.write("s" is Num)         // expect: false
io.write("s".type == String) // expect: true

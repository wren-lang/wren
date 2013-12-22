IO.write("s" is String)      // expect: true
IO.write("s" is Object)      // expect: true
IO.write("s" is Num)         // expect: false
IO.write("s".type == String) // expect: true

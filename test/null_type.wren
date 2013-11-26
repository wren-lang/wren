io.write(null is Null)      // expect: true
io.write(null is Object)    // expect: true
io.write(null is Bool)      // expect: false
io.write(null.type == Null) // expect: true

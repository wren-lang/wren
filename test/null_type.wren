IO.write(null is Null)      // expect: true
IO.write(null is Object)    // expect: true
IO.write(null is Bool)      // expect: false
IO.write(null.type == Null) // expect: true

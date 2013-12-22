IO.write(true is Bool)      // expect: true
IO.write(true is Object)    // expect: true
IO.write(true is Num)       // expect: false
IO.write(true.type == Bool) // expect: true

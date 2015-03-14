IO.print(true is Bool)      // expect: true
IO.print(true is Object)    // expect: true
IO.print(true is Num)       // expect: false
IO.print(true.type == Bool) // expect: true

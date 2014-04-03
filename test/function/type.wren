IO.print(new Fn { 0 } is Fn)      // expect: true
IO.print(new Fn { 0 } is Object)  // expect: true
IO.print(new Fn { 0 } is String)  // expect: false
IO.print(new Fn { 0 }.type == Fn) // expect: true

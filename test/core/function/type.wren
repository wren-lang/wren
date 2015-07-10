IO.print(Fn.new { 0 } is Fn)      // expect: true
IO.print(Fn.new { 0 } is Object)  // expect: true
IO.print(Fn.new { 0 } is String)  // expect: false
IO.print(Fn.new { 0 }.type == Fn) // expect: true

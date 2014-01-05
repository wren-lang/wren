IO.print((fn 0) is Function)      // expect: true
IO.print((fn 0) is Object)        // expect: true
IO.print((fn 0) is String)        // expect: false
IO.print((fn 0).type == Function) // expect: true

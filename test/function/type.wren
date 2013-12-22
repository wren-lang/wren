IO.write((fn 0) is Function)      // expect: true
IO.write((fn 0) is Object)        // expect: true
IO.write((fn 0) is String)        // expect: false
IO.write((fn 0).type == Function) // expect: true

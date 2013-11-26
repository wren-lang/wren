io.write((fn 0) is Function)      // expect: true
io.write((fn 0) is Object)        // expect: true
io.write((fn 0) is String)        // expect: false
io.write((fn 0).type == Function) // expect: true

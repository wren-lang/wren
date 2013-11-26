io.write([] is List)      // expect: true
io.write([] is Object)    // expect: true
io.write([] is Bool)      // expect: false
io.write([].type == List) // expect: true

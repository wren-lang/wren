IO.write([] is List)      // expect: true
IO.write([] is Object)    // expect: true
IO.write([] is Bool)      // expect: false
IO.write([].type == List) // expect: true

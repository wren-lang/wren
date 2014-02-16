IO.print([] is List)      // expect: true
IO.print([] is Sequence)  // expect: true
IO.print([] is Object)    // expect: true
IO.print([] is Bool)      // expect: false
IO.print([].type == List) // expect: true

System.print([] is List)      // expect: true
System.print([] is Sequence)  // expect: true
System.print([] is Object)    // expect: true
System.print([] is Bool)      // expect: false
System.print([].type == List) // expect: true

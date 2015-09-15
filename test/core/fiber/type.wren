var fiber = Fiber.new {}
System.print(fiber is Fiber)      // expect: true
System.print(fiber is Object)     // expect: true
System.print(fiber is Bool)       // expect: false
System.print(fiber.type == Fiber) // expect: true

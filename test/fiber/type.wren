var fiber = Fiber.create {}
IO.print(fiber is Fiber)      // expect: true
IO.print(fiber is Object)     // expect: true
IO.print(fiber is Bool)       // expect: false
IO.print(fiber.type == Fiber) // expect: true

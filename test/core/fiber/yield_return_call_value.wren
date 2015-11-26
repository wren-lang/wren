var fiber = Fiber.new {
  System.print("fiber 1")
  System.print(Fiber.yield())
  System.print(Fiber.yield())
}

fiber()                 // expect: fiber 1
System.print("main 1")  // expect: main 1
fiber("call 1")         // expect: call 1
System.print("main 2")  // expect: main 2
fiber()                 // expect: null
System.print("main 3")  // expect: main 3

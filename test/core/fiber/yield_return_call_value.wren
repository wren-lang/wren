var fiber = Fiber.new {
  System.print("fiber 1")
  System.print(Fiber.yield())
  System.print(Fiber.yield())
}

fiber.call() // expect: fiber 1
System.print("main 1") // expect: main 1
fiber.call("call 1") // expect: call 1
System.print("main 2") // expect: main 2
fiber.call() // expect: null
System.print("main 3") // expect: main 3

var fiber = Fiber.new {
  System.print("1")
  Fiber.yield()
  System.print("2")
}

System.print(fiber.isDone)  // expect: false
fiber.call()            // expect: 1
System.print(fiber.isDone)  // expect: false
fiber.call()            // expect: 2
System.print(fiber.isDone)  // expect: true

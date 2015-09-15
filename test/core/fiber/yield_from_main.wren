System.print("before") // expect: before
Fiber.yield()
System.print("not reached")

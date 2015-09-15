System.print("before") // expect: before
Fiber.yield(1)
System.print("not reached")
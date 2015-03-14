IO.print("before") // expect: before
Fiber.yield()
IO.print("not reached")

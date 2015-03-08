IO.print("before") // expect: before
Fiber.yield(1)
IO.print("not reached")
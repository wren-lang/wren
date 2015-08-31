var fiber = Fiber.new {
  IO.print("fiber 1")
  Fiber.yield("yield 1")
  IO.print("fiber 2")
  Fiber.yield("yield 2")
  IO.print("fiber 3")
}

IO.print(fiber.call())  // expect: fiber 1
                        // expect: yield 1
IO.print(fiber.call())  // expect: fiber 2
                        // expect: yield 2
IO.print(fiber.call())  // expect: fiber 3
                        // expect: null

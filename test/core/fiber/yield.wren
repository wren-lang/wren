var fiber = Fiber.new {
  IO.print("fiber 1")
  Fiber.yield()
  IO.print("fiber 2")
  Fiber.yield()
  IO.print("fiber 3")
}

fiber.call()          // expect: fiber 1
IO.print("main 1")    // expect: main 1
fiber.call()          // expect: fiber 2
IO.print("main 2")    // expect: main 2
fiber.call()          // expect: fiber 3
IO.print("main 3")    // expect: main 3

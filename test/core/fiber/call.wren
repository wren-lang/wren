var fiber = Fiber.new {
  IO.print("fiber")
}

IO.print("before") // expect: before
fiber.call()       // expect: fiber
IO.print("after")  // expect: after

var fiber = new Fiber {
  IO.print("fiber")
}

IO.print("before")    // expect: before
IO.print(fiber.try()) // expect: fiber
                      // expect: null
IO.print("after")     // expect: after

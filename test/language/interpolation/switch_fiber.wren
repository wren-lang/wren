var fiber = Fiber.new {
  System.print("in fiber")
  Fiber.yield("result")
}

System.print("outer %(fiber.call()) string")
// expect: in fiber
// expect: outer result string

var fiber = Fiber.create {
  IO.print("1")
  Fiber.yield
  IO.print("2")
}

IO.print(fiber.isDone)  // expect: false
fiber.run               // expect: 1
IO.print(fiber.isDone)  // expect: false
fiber.run               // expect: 2
IO.print(fiber.isDone)  // expect: true

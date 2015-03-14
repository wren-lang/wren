var fiber = new Fiber {
  IO.print("1")
  Fiber.yield()
  IO.print("2")
}

IO.print(fiber.isDone)  // expect: false
fiber.call()            // expect: 1
IO.print(fiber.isDone)  // expect: false
fiber.call()            // expect: 2
IO.print(fiber.isDone)  // expect: true

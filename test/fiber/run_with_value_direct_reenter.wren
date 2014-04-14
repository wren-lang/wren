var fiber

fiber = new Fiber {
  IO.print(1) // expect: 1
  fiber.run("ignored")
  IO.print(2) // expect: 2
}

fiber.call
IO.print(3) // expect: 3
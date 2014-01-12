var fiber = Fiber.create(fn {
  IO.print("fiber 1")
  Fiber.yield
  IO.print("fiber 2")
  Fiber.yield
  IO.print("fiber 3")
})

var result = fiber.run // expect: fiber 1
IO.print("main 1") // expect: main 1
result = fiber.run // expect: fiber 2
IO.print("main 2") // expect: main 2
result = fiber.run // expect: fiber 3
IO.print("main 3") // expect: main 3

var fiber = new Fiber {
  IO.print("fiber 1")
  var result = Fiber.yield
  IO.print(result)
  result = Fiber.yield
  IO.print(result)
}

fiber.run // expect: fiber 1
IO.print("main 1") // expect: main 1
fiber.run("run 1") // expect: run 1
IO.print("main 2") // expect: main 2
fiber.run // expect: null
IO.print("main 3") // expect: main 3

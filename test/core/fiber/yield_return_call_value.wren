var fiber = Fiber.new {
  IO.print("fiber 1")
  var result = Fiber.yield()
  IO.print(result)
  result = Fiber.yield()
  IO.print(result)
}

fiber.call() // expect: fiber 1
IO.print("main 1") // expect: main 1
fiber.call("call 1") // expect: call 1
IO.print("main 2") // expect: main 2
fiber.call() // expect: null
IO.print("main 3") // expect: main 3

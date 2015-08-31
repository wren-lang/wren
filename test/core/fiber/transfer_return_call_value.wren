var main = Fiber.current

var fiber = Fiber.new {
  IO.print("fiber 1")
  IO.print(main.transfer())

  // Yield to bounce back to main and clear the caller so we don't get a
  // double call() error below.
  Fiber.yield()

  IO.print(main.transfer())
}

fiber.transfer() // expect: fiber 1
IO.print("main 1") // expect: main 1
fiber.call("call 1") // expect: call 1

IO.print("main 2") // expect: main 2
// Transfer back into the fiber so it has a NULL caller.
fiber.transfer()

fiber.call() // expect: null
IO.print("main 3") // expect: main 3

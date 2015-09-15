var main = Fiber.current

var fiber = Fiber.new {
  System.print("fiber")
  System.print(main.transfer())
}

fiber.transfer() // expect: fiber
System.print("main") // expect: main
fiber.transfer("transfer") // expect: transfer

// This does not get run since we exit when the transferred fiber completes.
System.print("nope")

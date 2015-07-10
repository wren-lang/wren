var a = Fiber.new {
  IO.print("run")
}

// Run a through an intermediate fiber since it will get discarded and we need
// to return to the main one after a completes.
var b = Fiber.new {
  a.run()
  IO.print("nope")
}

b.call() // expect: run
a.run()  // expect runtime error: Cannot run a finished fiber.

var a = Fiber.new {
  IO.print("before") // expect: before
  Fiber.yield()
}

// Run a chain of fibers. Since none of them are called, they all get discarded
// and there is no remaining caller.
var b = Fiber.new { a.run() }
var c = Fiber.new { b.run() }
c.run()
IO.print("not reached")
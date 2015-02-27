var a = new Fiber {
  Fiber.yield(1) // expect runtime error: No fiber to yield to.
}

// Run a chain of fibers. Since none of them are called, they all get discarded
// and there is no remaining caller.
var b = new Fiber { a.run() }
var c = new Fiber { b.run() }
c.run()

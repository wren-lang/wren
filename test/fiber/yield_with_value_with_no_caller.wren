var a = new Fiber {
  IO.print("before") // expect: before
  Fiber.yield(1)
}

// Run a chain of fibers. Since none of them are called, they all get discarded
// and there is no remaining caller.
var b = new Fiber { a.run() }
var c = new Fiber { b.run() }
c.run()
IO.print("not reached")

var a = Fiber.new {
  IO.print("before") // expect: before
  Fiber.yield(1)
  IO.print("not reached")
}

// Transfer through a chain of fibers. Since none of them are called, they all
// get discarded and there is no remaining caller.
var b = Fiber.new { a.transfer() }
var c = Fiber.new { b.transfer() }
c.transfer()
IO.print("not reached")

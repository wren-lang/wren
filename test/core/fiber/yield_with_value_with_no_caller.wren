var a = Fiber.new {
  System.print("before") // expect: before
  Fiber.yield(1)
  System.print("not reached")
}

// Transfer through a chain of fibers. Since none of them are called, they all
// get discarded and there is no remaining caller.
var b = Fiber.new { a.transfer() }
var c = Fiber.new { b.transfer() }
c.transfer()
System.print("not reached")

var a = Fiber.new {
  IO.print("a")
}

var b = Fiber.new {
  IO.print("b before")
  a.transfer()
  IO.print("b after")
}

var c = Fiber.new {
  IO.print("c before")
  b.transfer()
  IO.print("c after")
}

IO.print("start") // expect: start

c.transfer()
// expect: c before
// expect: b before
// expect: a

// Nothing else gets run since the interpreter stops after a completes.

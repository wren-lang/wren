var a = Fiber.new {
  System.print("a")
}

var b = Fiber.new {
  System.print("b before")
  a.transfer("ignored")
  System.print("b after")
}

var c = Fiber.new {
  System.print("c before")
  b.transfer("ignored")
  System.print("c after")
}

System.print("start") // expect: start

c.transfer("ignored")
// expect: c before
// expect: b before
// expect: a

// Nothing else gets run since the interpreter stops after a completes.

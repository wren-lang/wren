var a = Fiber.new {|param|
  System.print("a %(param)")
}

var b = Fiber.new {|param|
  System.print("b before %(param)")
  a.transfer()
  System.print("b after")
}

var c = Fiber.new {|param|
  System.print("c before %(param)")
  b.transfer()
  System.print("c after")
}

System.print("start") // expect: start

c.transfer()
// expect: c before null
// expect: b before null
// expect: a null

// Nothing else gets run since the interpreter stops after a completes.

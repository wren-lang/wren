var a = Fiber.new {|param|
  System.print("a %(param)")
}

var b = Fiber.new {|param|
  System.print("b before %(param)")
  a.transfer("from b")
  System.print("b after")
}

var c = Fiber.new {|param|
  System.print("c before %(param)")
  b.transfer("from c")
  System.print("c after")
}

System.print("start") // expect: start

c.transfer("from main")
// expect: c before from main
// expect: b before from c
// expect: a from b

// Nothing else gets run since the interpreter stops after a completes.

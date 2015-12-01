var a = Fiber.new {
  System.print("run")
}

a() // expect: run
a.transfer("blah") // expect runtime error: Cannot transfer to a finished fiber.

var a = Fiber.new {
  System.print("run")
}

a.call() // expect: run
a.transfer() // expect runtime error: Cannot transfer to a finished fiber.

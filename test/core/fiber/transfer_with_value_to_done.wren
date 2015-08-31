var a = Fiber.new {
  IO.print("run")
}

a.call() // expect: run
a.transfer("blah") // expect runtime error: Cannot transfer to a finished fiber.

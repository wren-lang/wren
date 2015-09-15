var a = Fiber.new {
  Fiber.abort("Error!")
  System.print("should not get here")
}

a.try()
a.transfer("blah") // expect runtime error: Cannot transfer to an aborted fiber.

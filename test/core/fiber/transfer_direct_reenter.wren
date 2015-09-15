var F = Fiber.new {
  System.print(1)            // expect: 1
  System.print(F.transfer()) // expect: null
  System.print(2)            // expect: 2
}

F.call()
// F remembers its original caller so transfers back to main.
System.print(3) // expect: 3

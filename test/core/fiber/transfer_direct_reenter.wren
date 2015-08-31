var F = Fiber.new {
  IO.print(1)            // expect: 1
  IO.print(F.transfer()) // expect: null
  IO.print(2)            // expect: 2
}

F.call()
// F remembers its original caller so transfers back to main.
IO.print(3) // expect: 3

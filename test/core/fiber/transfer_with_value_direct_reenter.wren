var F = Fiber.new {
  System.print(1) // expect: 1
  System.print(F.transfer("value")) // expect: value
  System.print(2) // expect: 2
}

F.call()
System.print(3) // expect: 3

var f = Fiber.new {
  System.print(1) // expect: 1
  System.print(f.transfer("value")) // expect: value
  System.print(2) // expect: 2
}

f()
System.print(3) // expect: 3

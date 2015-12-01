var a = Fiber.new {
  System.print(2)
  b.transfer("ignored")
  System.print("nope")
}

var b = Fiber.new {
  System.print(1)
  a.transfer("ignored")
  System.print(3)
}

b()
// expect: 1
// expect: 2
// expect: 3
System.print(4) // expect: 4

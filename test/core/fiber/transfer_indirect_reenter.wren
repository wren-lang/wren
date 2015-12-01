var a = Fiber.new {
  System.print(2)
  b.transfer()
  System.print("nope")
}

var b = Fiber.new {
  System.print(1)
  a.transfer()
  System.print(3)
}

b()
// expect: 1
// expect: 2
// expect: 3
// b remembers its original caller so returns to main.
System.print(4) // expect: 4

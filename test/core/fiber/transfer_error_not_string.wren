var a = Fiber.new {
  b.transferError(123)
}

var b = Fiber.new {
  a.transfer()
}

b.try()
System.print(b.error) // expect: 123

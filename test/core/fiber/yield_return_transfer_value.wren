var main = Fiber.current

var a = Fiber.new {
  System.print("a")
  System.print(Fiber.yield())
}

var b = Fiber.new {
  System.print("b")
  System.print(Fiber.yield())

  a()
  a.transfer("value")
}

b() // expect: b
b.transfer()
// expect: null
// expect: a
// expect: value

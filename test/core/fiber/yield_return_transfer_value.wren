var main = Fiber.current

var a = Fiber.new {
  IO.print("a")
  IO.print(Fiber.yield())
}

var b = Fiber.new {
  IO.print("b")
  IO.print(Fiber.yield())

  a.call()
  a.transfer("value")
}

b.call() // expect: b
b.transfer()
// expect: null
// expect: a
// expect: value

var a
var b

a = Fiber.new {
  IO.print(2)
  b.run()
  IO.print("nope")
}

b = Fiber.new {
  IO.print(1)
  a.run()
  IO.print(3)
}

b.call()
// expect: 1
// expect: 2
// expect: 3
IO.print(4) // expect: 4

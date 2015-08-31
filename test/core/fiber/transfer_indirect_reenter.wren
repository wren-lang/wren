var A = Fiber.new {
  IO.print(2)
  B.transfer()
  IO.print("nope")
}

var B = Fiber.new {
  IO.print(1)
  A.transfer()
  IO.print(3)
}

B.call()
// expect: 1
// expect: 2
// expect: 3
// B remembers its original caller so returns to main.
IO.print(4) // expect: 4

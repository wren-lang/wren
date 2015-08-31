var A = Fiber.new {
  IO.print(2)
  B.transfer("ignored")
  IO.print("nope")
}

var B = Fiber.new {
  IO.print(1)
  A.transfer("ignored")
  IO.print(3)
}

B.call()
// expect: 1
// expect: 2
// expect: 3
IO.print(4) // expect: 4

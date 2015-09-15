var A = Fiber.new {
  System.print(2)
  B.transfer()
  System.print("nope")
}

var B = Fiber.new {
  System.print(1)
  A.transfer()
  System.print(3)
}

B.call()
// expect: 1
// expect: 2
// expect: 3
// B remembers its original caller so returns to main.
System.print(4) // expect: 4

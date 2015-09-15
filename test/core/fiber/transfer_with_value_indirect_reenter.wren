var A = Fiber.new {
  System.print(2)
  B.transfer("ignored")
  System.print("nope")
}

var B = Fiber.new {
  System.print(1)
  A.transfer("ignored")
  System.print(3)
}

B.call()
// expect: 1
// expect: 2
// expect: 3
System.print(4) // expect: 4

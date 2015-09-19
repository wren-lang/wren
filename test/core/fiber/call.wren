var fiber = Fiber.new {
  System.print("fiber")
}

System.print("before") // expect: before
fiber.call()       // expect: fiber
System.print("after")  // expect: after

var fiber2 = Fiber.new {
  System.print("fiber2")
}

System.print("before") // expect: before
fiber2.()       // expect: fiber2
System.print("after")  // expect: after

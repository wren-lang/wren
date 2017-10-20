var fiber = Fiber.new {|value|
  System.print(value)
}

System.print("before") // expect: before
fiber.call()           // expect: null
System.print("after")  // expect: after

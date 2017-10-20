var fiber = Fiber.new {|value|
  System.print(value)
}

System.print("before") // expect: before
fiber.call("value")    // expect: value
System.print("after")  // expect: after

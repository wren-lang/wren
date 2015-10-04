var fiber = Fiber.new {
  Fiber.abort(null)
  System.print("get here") // expect: get here
  Fiber.yield("value")
}

System.print(fiber.try()) // expect: value
System.print(fiber.isDone) // expect: false
System.print(fiber.error) // expect: null

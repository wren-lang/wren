var fiber = Fiber.new {
  System.print("called")
  System.print(Fiber.yield())
  System.print("transferred")
}

fiber.call()     // expect: called
fiber.transfer() // expect: null
                 // expect: transferred

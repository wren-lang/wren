var main = Fiber.current

var fiber = Fiber.new {
  System.print("transferred")
  System.print(main.transfer())
  System.print("called")
}

fiber.transfer() // expect: transferred
System.print("main") // expect: main
fiber.call()     // expect: null
                 // expect: called

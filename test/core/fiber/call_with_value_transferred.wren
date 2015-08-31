var main = Fiber.current

var fiber = Fiber.new {
  IO.print("transferred")
  IO.print(main.transfer())
  IO.print("called")
}

fiber.transfer()    // expect: transferred
IO.print("main")    // expect: main
fiber.call("value") // expect: value
                    // expect: called

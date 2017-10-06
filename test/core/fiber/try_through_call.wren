// A runtime error percolates through the fiber call chain until it hits a
// try(). Every intermediate fiber is aborted too.

var fiber1 = Fiber.new {
  System.print("1 before")
  Fiber.abort("Abort!")
  System.print("1 after")
}

var fiber2 = Fiber.new {
  System.print("2 before")
  fiber1.call()
  System.print("2 after")
}

var fiber3 = Fiber.new {
  System.print("3 before")
  fiber2.call()
  System.print("3 after")
}

var fiber4 = Fiber.new {
  System.print("4 before")
  fiber3.try()
  System.print("4 after")
}

var fiber5 = Fiber.new {
  System.print("5 before")
  fiber4.call()
  System.print("5 after")
}

fiber5.call()
// expect: 5 before
// expect: 4 before
// expect: 3 before
// expect: 2 before
// expect: 1 before
// expect: 4 after
// expect: 5 after
System.print("after") // expect: after

// The fibers between the aborted one and the tried one are all errored.
System.print(fiber1.error) // expect: Abort!
System.print(fiber2.error) // expect: Abort!
System.print(fiber3.error) // expect: Abort!
System.print(fiber4.error) // expect: null
System.print(fiber5.error) // expect: null

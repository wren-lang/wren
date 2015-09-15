var fiber = Fiber.new {
  System.print("fiber")
}

// The first value passed to the fiber is ignored, since there's no yield call
// to return it.
System.print("before")    // expect: before
fiber.call("ignored") // expect: fiber
System.print("after")     // expect: after

var fiber = Fiber.new {
  IO.print("fiber")
}

// The first value passed to the fiber is ignored, since there's no yield call
// to return it.
IO.print("before")    // expect: before
fiber.call("ignored") // expect: fiber
IO.print("after")     // expect: after

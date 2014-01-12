var fiber = Fiber.create(fn {
  IO.print("fiber")
})

IO.print("before") // expect: before
fiber.run          // expect: fiber
IO.print("after")  // expect: after

// TODO: Test handles error if fiber tries to run itself.
// TODO: Test create is passed right argument type.
// TODO: Test closing over stuff in fiber function.
// TODO: Test running a finished fiber.

var fiber
var closure

{
  var a = "before"
  fiber = Fiber.create(fn {
    IO.print(a)
    Fiber.yield
    a = "after"
    Fiber.yield
    IO.print(a)
    a = "final"
  })

  closure = fn {
    IO.print(a)
  }
}

fiber.run    // expect: before
closure.call // expect: before
fiber.run
closure.call // expect: after
fiber.run    // expect: after
closure.call // expect: final

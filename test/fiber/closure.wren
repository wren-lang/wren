var fiber
var closure

{
  var a = "before"
  fiber = new Fiber {
    IO.print(a)
    Fiber.yield
    a = "after"
    Fiber.yield
    IO.print(a)
    a = "final"
  }

  closure = new Fn {
    IO.print(a)
  }
}

fiber.run    // expect: before
closure.call // expect: before
fiber.run
closure.call // expect: after
fiber.run    // expect: after
closure.call // expect: final

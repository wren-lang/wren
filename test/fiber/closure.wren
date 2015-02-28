var fiber
var closure

{
  var a = "before"
  fiber = new Fiber {
    IO.print(a)
    Fiber.yield()
    a = "after"
    Fiber.yield()
    IO.print(a)
    a = "final"
  }

  closure = new Fn {
    IO.print(a)
  }
}

fiber.call()   // expect: before
closure.call() // expect: before
fiber.call()
closure.call() // expect: after
fiber.call()   // expect: after
closure.call() // expect: final

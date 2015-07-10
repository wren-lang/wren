var fiber
var closure

{
  var a = "before"
  fiber = Fiber.new {
    IO.print(a)
    Fiber.yield()
    a = "after"
    Fiber.yield()
    IO.print(a)
    a = "final"
  }

  closure = Fn.new {
    IO.print(a)
  }
}

fiber.call()   // expect: before
closure.call() // expect: before
fiber.call()
closure.call() // expect: after
fiber.call()   // expect: after
closure.call() // expect: final

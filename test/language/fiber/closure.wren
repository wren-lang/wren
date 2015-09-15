var fiber
var closure

{
  var a = "before"
  fiber = Fiber.new {
    System.print(a)
    Fiber.yield()
    a = "after"
    Fiber.yield()
    System.print(a)
    a = "final"
  }

  closure = Fn.new {
    System.print(a)
  }
}

fiber.call()   // expect: before
closure.call() // expect: before
fiber.call()
closure.call() // expect: after
fiber.call()   // expect: after
closure.call() // expect: final

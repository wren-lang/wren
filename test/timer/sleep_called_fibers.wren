import "timer" for Timer

var a = Fiber.new {
  IO.print("a before")
  Timer.sleep(10)
  IO.print("a after")
}

var b = Fiber.new {
  IO.print("b before")
  a.call()
  IO.print("b after")
}

// All fibers are suspended since they were directly called and not scheduled.
IO.print("before")  // expect: before
b.call()            // expect: b before
                    // expect: a before
                    // expect: a after
                    // expect: b after
IO.print("done")    // expect: done

import "timer" for Timer

var a = Fiber.new {
  System.print("a before")
  Timer.sleep(10)
  System.print("a after")
}

var b = Fiber.new {
  System.print("b before")
  a.call()
  System.print("b after")
}

// All fibers are suspended since they were directly called and not scheduled.
System.print("before")  // expect: before
b.call()            // expect: b before
                    // expect: a before
                    // expect: a after
                    // expect: b after
System.print("done")    // expect: done

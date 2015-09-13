import "scheduler" for Scheduler
import "timer" for Timer

Scheduler.add {
  IO.print("a before")
  Timer.sleep(0)
  IO.print("a after")
}

Scheduler.add {
  IO.print("b before")
  Timer.sleep(0)
  IO.print("b after")
}

IO.print("main")
Timer.sleep(0) // This is enough to let the other fibers run to their sleep.
IO.print("main after")
Timer.sleep(0) // And now their sleeps complete.
IO.print("done")

// expect: main
// expect: a before
// expect: b before
// expect: main after
// expect: a after
// expect: b after
// expect: done

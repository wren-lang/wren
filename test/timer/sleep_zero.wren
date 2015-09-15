import "scheduler" for Scheduler
import "timer" for Timer

Scheduler.add {
  System.print("a before")
  Timer.sleep(0)
  System.print("a after")
}

Scheduler.add {
  System.print("b before")
  Timer.sleep(0)
  System.print("b after")
}

System.print("main")
Timer.sleep(0) // This is enough to let the other fibers run to their sleep.
System.print("main after")
Timer.sleep(0) // And now their sleeps complete.
System.print("done")

// expect: main
// expect: a before
// expect: b before
// expect: main after
// expect: a after
// expect: b after
// expect: done

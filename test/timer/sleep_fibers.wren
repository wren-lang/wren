import "scheduler" for Scheduler
import "timer" for Timer

Scheduler.add {
  Timer.sleep(2)
  IO.print("a")
}

Scheduler.add {
  Timer.sleep(1)
  IO.print("b")
}

IO.print("main")
Timer.sleep(3)
IO.print("done")
// expect: main
// expect: b
// expect: a
// expect: done

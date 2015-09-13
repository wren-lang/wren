import "scheduler" for Scheduler
import "timer" for Timer

// These are both rounded to 1, so "a" should complete first.
Scheduler.add {
  Timer.sleep(1.5)
  IO.print("a")
}

Scheduler.add {
  Timer.sleep(1.3)
  IO.print("b")
}

IO.print("main")
Timer.sleep(3)
IO.print("done")
// expect: main
// expect: a
// expect: b
// expect: done

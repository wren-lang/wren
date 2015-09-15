import "scheduler" for Scheduler
import "timer" for Timer

// These are both rounded to 1, so "a" should complete first.
Scheduler.add {
  Timer.sleep(1.5)
  System.print("a")
}

Scheduler.add {
  Timer.sleep(1.3)
  System.print("b")
}

System.print("main")
Timer.sleep(3)
System.print("done")
// expect: main
// expect: a
// expect: b
// expect: done

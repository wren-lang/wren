import "scheduler" for Scheduler
import "timer" for Timer

Scheduler.add {
  Timer.sleep(2)
  System.print("a")
}

Scheduler.add {
  Timer.sleep(1)
  System.print("b")
}

System.print("main")
Timer.sleep(3)
System.print("done")
// expect: main
// expect: b
// expect: a
// expect: done

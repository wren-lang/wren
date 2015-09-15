import "scheduler" for Scheduler
import "timer" for Timer

var run = false
Scheduler.add {
  run = true
}

// Does not run immediately.
System.print(run) // expect: false

Timer.sleep(0)

// Runs when the main fiber suspends on IO.
System.print(run) // expect: true

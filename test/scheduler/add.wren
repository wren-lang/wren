import "scheduler" for Scheduler
import "timer" for Timer

var run = false
Scheduler.add {
  run = true
}

// Does not run immediately.
IO.print(run) // expect: false

Timer.sleep(0)

// Runs when the main fiber suspends on IO.
IO.print(run) // expect: true

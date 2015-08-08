import "timer" for Timer

Fiber.new {
  Timer.sleep(0)
  IO.print("a")
}.call()

Fiber.new {
  Timer.sleep(0)
  IO.print("b")
}.call()

Fiber.new {
  Timer.sleep(0)
  IO.print("c")
}.call()

IO.print("main")
Timer.sleep(0) // This is enough to let the other fiber run.
IO.print("done")

// expect: main
// Run in the order they were enqueued.
// expect: a
// expect: b
// expect: c
// expect: done

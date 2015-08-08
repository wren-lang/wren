import "timer" for Timer

Fiber.new {
  Timer.sleep(2)
  IO.print("a")
}.call()

Fiber.new {
  Timer.sleep(1)
  IO.print("b")
}.call()

IO.print("main")
Timer.sleep(3)
IO.print("done")
// expect: main
// expect: b
// expect: a
// expect: done

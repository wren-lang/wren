class Timer {
  static sleep(milliseconds) {
    if (!(milliseconds is Num)) Fiber.abort("Milliseconds must be a number.")
    if (milliseconds < 0) Fiber.abort("Milliseconds cannot be negative.")
    startTimer_(milliseconds, Fiber.current)

    runNextScheduled_()
  }

  // TODO: Once the CLI modules are more fleshed out, find a better place to
  // put this.
  static schedule(callable) {
    if (__scheduled == null) __scheduled = []
    __scheduled.add(Fiber.new {
      callable.call()
      runNextScheduled_()
    })
  }

  foreign static startTimer_(milliseconds, fiber)

  // Called by native code.
  static resumeTimer_(fiber) {
    fiber.transfer()
  }

  static runNextScheduled_() {
    if (__scheduled == null || __scheduled.isEmpty) {
      Fiber.suspend()
    } else {
      __scheduled.removeAt(0).transfer()
    }
  }
}

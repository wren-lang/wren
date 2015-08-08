class Timer {
  static sleep(milliseconds) {
    if (!(milliseconds is Num)) Fiber.abort("Milliseconds must be a number.")
    if (milliseconds < 0) Fiber.abort("Milliseconds cannot be negative.")
    startTimer_(milliseconds, Fiber.current)
    Fiber.yield()
  }

  foreign static startTimer_(milliseconds, fiber)

  // Called by native code.
  static resumeTimer_(fiber) {
    fiber.run()
  }
}

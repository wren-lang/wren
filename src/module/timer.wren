class Timer {
  static sleep(milliseconds) {
    startTimer_(milliseconds, Fiber.current)
    Fiber.yield()
  }

  foreign static startTimer_(milliseconds, fiber)

  // Called by native code.
  static resumeTimer_(fiber) {
    fiber.run()
  }
}

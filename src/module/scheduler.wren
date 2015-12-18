class Scheduler {
  static def add(callable) {
    if (__scheduled == null) __scheduled = []

    __scheduled.add(Fiber.new {
      callable.call()
      runNextScheduled_()
    })
  }

  // Called by native code.
  static def resume_(fiber) { fiber.transfer() }
  static def resume_(fiber, arg) { fiber.transfer(arg) }
  static def resumeError_(fiber, error) { fiber.transferError(error) }

  static def runNextScheduled_() {
    if (__scheduled == null || __scheduled.isEmpty) {
      return Fiber.suspend()
    } else {
      return __scheduled.removeAt(0).transfer()
    }
  }

  foreign static def captureMethods_()
}

Scheduler.captureMethods_()

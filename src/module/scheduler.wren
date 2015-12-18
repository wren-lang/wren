class Scheduler {
  def static add(callable) {
    if (__scheduled == null) __scheduled = []

    __scheduled.add(Fiber.new {
      callable.call()
      runNextScheduled_()
    })
  }

  // Called by native code.
  def static resume_(fiber) { fiber.transfer() }
  def static resume_(fiber, arg) { fiber.transfer(arg) }
  def static resumeError_(fiber, error) { fiber.transferError(error) }

  def static runNextScheduled_() {
    if (__scheduled == null || __scheduled.isEmpty) {
      return Fiber.suspend()
    } else {
      return __scheduled.removeAt(0).transfer()
    }
  }

  def foreign static captureMethods_()
}

Scheduler.captureMethods_()

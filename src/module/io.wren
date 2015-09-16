import "scheduler" for Scheduler

class File {
  static size(path) {
    if (!(path is String)) Fiber.abort("Path must be a string.")

    startSize_(path, Fiber.current)
    var result = Scheduler.runNextScheduled_()
    if (result is String) Fiber.abort(result)

    return result
  }

  foreign static startSize_(path, fiber)
}

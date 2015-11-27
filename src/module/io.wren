import "scheduler" for Scheduler

foreign class File {
  static open(path) {
    if (!(path is String)) Fiber.abort("Path must be a string.")

    open_(path, Fiber.current)
    var fd = Scheduler.runNextScheduled_()
    return new_(fd)
  }

  static open(path, function) {
    var file = open(path)
    var fiber = Fiber.new { function(file) }

    // Poor man's finally. Can we make this more elegant?
    var result = fiber.try()
    file.close()

    // TODO: Want something like rethrow since now the callstack ends here. :(
    if (fiber.error != null) Fiber.abort(fiber.error)
    return result
  }

  static read(path) {
    return File.open(path) {|file| file.readBytes(file.size) }
  }

  static size(path) {
    if (!(path is String)) Fiber.abort("Path must be a string.")

    sizePath_(path, Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  construct new_(fd) {}

  close() {
    if (close_(Fiber.current)) return
    Scheduler.runNextScheduled_()
  }

  isOpen { descriptor != -1 }

  size {
    if (!isOpen) Fiber.abort("File is not open.")

    size_(Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  readBytes(count) {
    if (!isOpen) Fiber.abort("File is not open.")
    if (!(count is Num)) Fiber.abort("Count must be an integer.")
    if (!count.isInteger) Fiber.abort("Count must be an integer.")
    if (count < 0) Fiber.abort("Count cannot be negative.")

    readBytes_(count, Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  foreign static open_(path, fiber)
  foreign static sizePath_(path, fiber)

  foreign close_(fiber)
  foreign descriptor
  foreign readBytes_(count, fiber)
  foreign size_(fiber)
}

class Stdin {
  static readLine() {
    if (__isClosed == true) {
      Fiber.abort("Stdin was closed.")
    }

    // TODO: Error if other fiber is already waiting.
    readStart_()

    __waitingFiber = Fiber.current
    var line = Scheduler.runNextScheduled_()

    readStop_()
    return line
  }

  static onData_(data) {
    if (data == null) {
      __isClosed = true
      readStop_()

      if (__line != null) {
        var line = __line
        __line = null
        if (__waitingFiber != null) __waitingFiber.transfer(line)
      } else {
        __waitingFiber.transferError("Stdin was closed.")
      }
    }

    // TODO: Handle Windows line separators.
    var lineSeparator = data.indexOf("\n")

    if (__line == null) __line = ""
    if (lineSeparator == -1) {
      // No end of line yet, so just accumulate it.
      __line = __line + data
    } else {
      // Split the line at the separator.
      var line = __line + data[0...lineSeparator]
      if (lineSeparator > 0 && lineSeparator < data.count - 1) {
        // Buffer up the characters after the separator for the next line.
        __line = data[lineSeparator + 1..-1]
      } else {
        __line = ""
      }

      if (__waitingFiber != null) __waitingFiber.transfer(line)
    }
  }

  foreign static readStart_()
  foreign static readStop_()
}

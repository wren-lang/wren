import "scheduler" for Scheduler

class Directory {
  static def list(path) {
    if (!(path is String)) Fiber.abort("Path must be a string.")

    list_(path, Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  foreign static def list_(path, fiber)
}

foreign class File {
  static def open(path) {
    if (!(path is String)) Fiber.abort("Path must be a string.")

    open_(path, Fiber.current)
    var fd = Scheduler.runNextScheduled_()
    return new_(fd)
  }

  static def open(path, fn) {
    var file = open(path)
    var fiber = Fiber.new { fn.call(file) }

    // Poor man's finally. Can we make this more elegant?
    var result = fiber.try()
    file.close()

    // TODO: Want something like rethrow since now the callstack ends here. :(
    if (fiber.error != null) Fiber.abort(fiber.error)
    return result
  }

  static def read(path) {
    return File.open(path) {|file| file.readBytes(file.size) }
  }

  static def size(path) {
    if (!(path is String)) Fiber.abort("Path must be a string.")

    sizePath_(path, Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  static def stat(path) {
    if (!(path is String)) Fiber.abort("Path must be a string.")

    statPath_(path, Fiber.current)
    return Stat.new_(Scheduler.runNextScheduled_())
  }

  construct new_(fd) {}

  def close() {
    if (close_(Fiber.current)) return
    Scheduler.runNextScheduled_()
  }

  foreign def descriptor

  def isOpen { descriptor != -1 }

  def size {
    if (!isOpen) Fiber.abort("File is not open.")

    size_(Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  def readBytes(count) { readBytes(count, 0) }

  def readBytes(count, offset) {
    if (!isOpen) Fiber.abort("File is not open.")
    if (!(count is Num)) Fiber.abort("Count must be an integer.")
    if (!count.isInteger) Fiber.abort("Count must be an integer.")
    if (count < 0) Fiber.abort("Count cannot be negative.")

    if (!(offset is Num)) Fiber.abort("Offset must be an integer.")
    if (!offset.isInteger) Fiber.abort("Offset must be an integer.")
    if (offset < 0) Fiber.abort("Offset cannot be negative.")

    readBytes_(count, offset, Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  foreign static def open_(path, fiber)
  foreign static def sizePath_(path, fiber)
  foreign static def statPath_(path, fiber)

  foreign def close_(fiber)
  foreign def readBytes_(count, start, fiber)
  foreign def size_(fiber)
}

class Stat {
  construct new_(fields) {
    _fields = fields
  }

  def device { _fields[0] }
  def inode { _fields[1] }
  def mode { _fields[2] }
  def linkCount { _fields[3] }
  def user { _fields[4] }
  def group { _fields[5] }
  def specialDevice { _fields[6] }
  def size { _fields[7] }
  def blockSize { _fields[8] }
  def blockCount { _fields[9] }
}

class Stdin {
  static def readLine() {
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

  static def onData_(data) {
    if (data == null) {
      __isClosed = true
      readStop_()

      if (__line != null) {
        // Emit the last line.
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

  foreign static def readStart_()
  foreign static def readStop_()
}

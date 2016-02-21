import "scheduler" for Scheduler

class Directory {
  // TODO: Copied from File. Figure out good way to share this.
  static ensurePath_(path) {
    if (!(path is String)) Fiber.abort("Path must be a string.")
  }

  static list(path) {
    ensurePath_(path)
    list_(path, Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  foreign static list_(path, fiber)
}

foreign class File {
  static create(path) {
    return openWithFlags(path,
        FileFlags.writeOnly |
        FileFlags.create |
        FileFlags.truncate)
  }

  static create(path, fn) {
    return openWithFlags(path,
        FileFlags.writeOnly |
        FileFlags.create |
        FileFlags.truncate, fn)
  }

  static delete(path) {
    File.ensurePath_(path)
    delete_(path, Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  static open(path) { openWithFlags(path, FileFlags.readOnly) }

  static open(path, fn) { openWithFlags(path, FileFlags.readOnly, fn) }

  // TODO: Add named parameters and then call this "open(_,flags:_)"?
  // TODO: Test.
  static openWithFlags(path, flags) {
    File.ensurePath_(path)
    File.ensureInt_(flags, "Flags")
    open_(path, flags, Fiber.current)
    var fd = Scheduler.runNextScheduled_()
    return new_(fd)
  }

  static openWithFlags(path, flags, fn) {
    var file = openWithFlags(path, flags)
    var fiber = Fiber.new { fn.call(file) }

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
    File.ensurePath_(path)
    sizePath_(path, Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  construct new_(fd) {}

  close() {
    if (close_(Fiber.current)) return
    Scheduler.runNextScheduled_()
  }

  foreign descriptor

  isOpen { descriptor != -1 }

  size {
    ensureOpen_()
    size_(Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  readBytes(count) { readBytes(count, 0) }

  readBytes(count, offset) {
    ensureOpen_()
    File.ensureInt_(count, "Count")
    File.ensureInt_(offset, "Offset")

    readBytes_(count, offset, Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  writeBytes(bytes) { writeBytes(bytes, size) }

  writeBytes(bytes, offset) {
    ensureOpen_()
    if (!(bytes is String)) Fiber.abort("Bytes must be a string.")
    File.ensureInt_(offset, "Offset")

    writeBytes_(bytes, offset, Fiber.current)
    return Scheduler.runNextScheduled_()
  }

  ensureOpen_() {
    if (!isOpen) Fiber.abort("File is not open.")
  }

  static ensurePath_(path) {
    if (!(path is String)) Fiber.abort("Path must be a string.")
  }

  static ensureInt_(value, name) {
    if (!(value is Num)) Fiber.abort("%(name) must be an integer.")
    if (!value.isInteger) Fiber.abort("%(name) must be an integer.")
    if (value < 0) Fiber.abort("%(name) cannot be negative.")
  }

  foreign static delete_(path, fiber)
  foreign static open_(path, flags, fiber)
  foreign static sizePath_(path, fiber)

  foreign close_(fiber)
  foreign readBytes_(count, offset, fiber)
  foreign size_(fiber)
  foreign writeBytes_(bytes, offset, fiber)
}

class FileFlags {
  static readOnly  { 0x0000 }
  static writeOnly { 0x0001 }
  static readWrite { 0x0002 }
  static sync      { 0x0080 }
  static create    { 0x0200 }
  static truncate  { 0x0400 }
  static exclusive { 0x0800 }
}

class Stat {
  construct new_(fields) {
    _fields = fields
  }

  static path(path) {
    if (!(path is String)) Fiber.abort("Path must be a string.")

    path_(path, Fiber.current)
    return Stat.new_(Scheduler.runNextScheduled_())
  }

  device { _fields[0] }
  inode { _fields[1] }
  mode { _fields[2] }
  linkCount { _fields[3] }
  user { _fields[4] }
  group { _fields[5] }
  specialDevice { _fields[6] }
  size { _fields[7] }
  blockSize { _fields[8] }
  blockCount { _fields[9] }

  foreign static path_(path, fiber)
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

  foreign static readStart_()
  foreign static readStop_()
}

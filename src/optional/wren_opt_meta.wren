class Meta {
  static def eval(source) {
    if (!(source is String)) Fiber.abort("Source code must be a string.")

    var fn = compile_(source)
    // TODO: Include compile errors.
    if (fn == null) Fiber.abort("Could not compile source code.")

    Fiber.new(fn).call()
  }

  foreign static def compile_(source)
}

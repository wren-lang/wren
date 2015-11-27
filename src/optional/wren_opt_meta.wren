class Meta {
  static eval(source) {
    if (!(source is String)) Fiber.abort("Source code must be a string.")

    var function = compile_(source)
    // TODO: Include compile errors.
    if (function == null) Fiber.abort("Could not compile source code.")

    Fiber.new(function)()
  }

  foreign static compile_(source)
}

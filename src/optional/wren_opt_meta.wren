class Meta {
  static eval(source) {
    if (!(source is String)) Fiber.abort("Source code must be a string.")

    var fn = compile_(source, false, false)
    // TODO: Include compile errors.
    if (fn == null) Fiber.abort("Could not compile source code.")

    Fiber.new(fn).call()
  }

  static compileExpression(source) {
    if (!(source is String)) Fiber.abort("Source code must be a string.")
    return compile_(source, true, true)
  }

  foreign static compile_(source, isExpression, printErrors)
}

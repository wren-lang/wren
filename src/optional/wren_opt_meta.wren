class Meta {
  def static eval(source) {
    if (!(source is String)) Fiber.abort("Source code must be a string.")

    var fn = compile_(source)
    // TODO: Include compile errors.
    if (fn == null) Fiber.abort("Could not compile source code.")

    Fiber.new(fn).call()
  }

  def foreign static compile_(source)
}

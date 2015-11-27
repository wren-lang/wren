// Generated automatically from src/optional/wren_opt_meta.wren. Do not edit.
static const char* metaModuleSource =
"class Meta {\n"
"  static eval(source) {\n"
"    if (!(source is String)) Fiber.abort(\"Source code must be a string.\")\n"
"\n"
"    var function = compile_(source)\n"
"    // TODO: Include compile errors.\n"
"    if (function == null) Fiber.abort(\"Could not compile source code.\")\n"
"\n"
"    Fiber.new(function)()\n"
"  }\n"
"\n"
"  foreign static compile_(source)\n"
"}\n";

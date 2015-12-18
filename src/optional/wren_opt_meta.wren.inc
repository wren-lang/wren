// Generated automatically from src/optional/wren_opt_meta.wren. Do not edit.
static const char* metaModuleSource =
"class Meta {\n"
"  static def eval(source) {\n"
"    if (!(source is String)) Fiber.abort(\"Source code must be a string.\")\n"
"\n"
"    var fn = compile_(source)\n"
"    // TODO: Include compile errors.\n"
"    if (fn == null) Fiber.abort(\"Could not compile source code.\")\n"
"\n"
"    Fiber.new(fn).call()\n"
"  }\n"
"\n"
"  foreign static def compile_(source)\n"
"}\n";

// Generated automatically from src/module/io.wren. Do not edit.
static const char* ioModuleSource =
"import \"scheduler\" for Scheduler\n"
"\n"
"class File {\n"
"  static size(path) {\n"
"    if (!(path is String)) Fiber.abort(\"Path must be a string.\")\n"
"\n"
"    startSize_(path, Fiber.current)\n"
"    var result = Scheduler.runNextScheduled_()\n"
"    if (result is String) Fiber.abort(result)\n"
"\n"
"    return result\n"
"  }\n"
"\n"
"  foreign static startSize_(path, fiber)\n"
"}\n";

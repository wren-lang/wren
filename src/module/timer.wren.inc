// Generated automatically from src/module/timer.wren. Do not edit.
static const char* timerModuleSource =
"import \"scheduler\" for Scheduler\n"
"\n"
"class Timer {\n"
"  static sleep(milliseconds) {\n"
"    if (!(milliseconds is Num)) Fiber.abort(\"Milliseconds must be a number.\")\n"
"    if (milliseconds < 0) Fiber.abort(\"Milliseconds cannot be negative.\")\n"
"\n"
"    startTimer_(milliseconds, Fiber.current)\n"
"    Scheduler.runNextScheduled_()\n"
"  }\n"
"\n"
"  foreign static startTimer_(milliseconds, fiber)\n"
"}\n";

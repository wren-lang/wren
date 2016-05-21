// Generated automatically from src/module/os.wren. Do not edit.
static const char* osModuleSource =
"class Platform {\n"
"  foreign static isPosix\n"
"  foreign static name\n"
"\n"
"  static isWindows { name == \"Windows\" }\n"
"}\n"
"\n"
"class Process {\n"
"  // TODO: This will need to be smarter when wren supports CLI options.\n"
"  static arguments { allArguments[2..-1] }\n"
"\n"
"  foreign static allArguments\n"
"}\n";

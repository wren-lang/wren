#include <stdio.h>
#include <string.h>

#include "io.h"
#include "modules.h"
#include "path.h"
#include "scheduler.h"
#include "vm.h"

// The single VM instance that the CLI uses.
static WrenVM* vm;

static WrenBindForeignMethodFn bindMethodFn = NULL;
static WrenBindForeignClassFn bindClassFn = NULL;
static WrenForeignMethodFn afterLoadFn = NULL;

static uv_loop_t* loop;

// TODO: This isn't currently used, but probably will be when package imports
// are supported. If not then, then delete this.
static char* rootDirectory = NULL;

// The exit code to use unless some other error overrides it.
int defaultExitCode = 0;

// Reads the contents of the file at [path] and returns it as a heap allocated
// string.
//
// Returns `NULL` if the path could not be found. Exits if it was found but
// could not be read.
static char* readFile(const char* path)
{
  FILE* file = fopen(path, "rb");
  if (file == NULL) return NULL;
  
  // Find out how big the file is.
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);
  
  // Allocate a buffer for it.
  char* buffer = (char*)malloc(fileSize + 1);
  if (buffer == NULL)
  {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }
  
  // Read the entire file.
  size_t bytesRead = fread(buffer, 1, fileSize, file);
  if (bytesRead < fileSize)
  {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }
  
  // Terminate the string.
  buffer[bytesRead] = '\0';
  
  fclose(file);
  return buffer;
}

// Applies the CLI's import resolution policy. The rules are:
//
// * If [name] starts with "./" or "../", it is a relative import, relative to
//   [importer]. The resolved path is [name] concatenated onto the directory
//   containing [importer] and then normalized.
//
//   For example, importing "./a/./b/../c" from "d/e/f" gives you "d/e/a/c".
//
// * Otherwise, it is a "package" import. This isn't implemented yet.
//
static const char* resolveModule(WrenVM* vm, const char* importer,
                                 const char* name)
{
  size_t nameLength = strlen(name);
  
  // See if it's a relative import.
  if (nameLength > 2 &&
      ((name[0] == '.' && name[1] == '/') ||
       (name[0] == '.' && name[1] == '.' && name[2] == '/')))
  {
    // Get the directory containing the importing module.
    Path* relative = pathNew(importer);
    pathDirName(relative);
    
    // Add the relative import path.
    pathJoin(relative, name);
    Path* normal = pathNormalize(relative);
    pathFree(relative);

    char* resolved = pathToString(normal);
    pathFree(normal);
    return resolved;
  }
  else
  {
    // TODO: Implement package imports. For now, treat any non-relative import
    // as an import relative to the current working directory.
  }
  
  return name;
}

// Attempts to read the source for [module] relative to the current root
// directory.
//
// Returns it if found, or NULL if the module could not be found. Exits if the
// module was found but could not be read.
static char* readModule(WrenVM* vm, const char* module)
{
  // Since the module has already been resolved, it should now be either a
  // valid relative path, or a package-style name.
  
  // TODO: Implement package imports.
  
  // Add a ".wren" file extension.
  Path* modulePath = pathNew(module);
  pathAppendString(modulePath, ".wren");
  
  char* source = readFile(modulePath->chars);
  pathFree(modulePath);
  
  if (source != NULL) return source;

  // TODO: This used to look for a file named "<path>/module.wren" if
  // "<path>.wren" could not be found. Do we still want to support that with
  // the new relative import and package stuff?

  // Otherwise, see if it's a built-in module.
  return readBuiltInModule(module);
}

// Binds foreign methods declared in either built in modules, or the injected
// API test modules.
static WrenForeignMethodFn bindForeignMethod(WrenVM* vm, const char* module,
    const char* className, bool isStatic, const char* signature)
{
  WrenForeignMethodFn method = bindBuiltInForeignMethod(vm, module, className,
                                                        isStatic, signature);
  if (method != NULL) return method;
  
  if (bindMethodFn != NULL)
  {
    return bindMethodFn(vm, module, className, isStatic, signature);
  }

  return NULL;
}

// Binds foreign classes declared in either built in modules, or the injected
// API test modules.
static WrenForeignClassMethods bindForeignClass(
    WrenVM* vm, const char* module, const char* className)
{
  WrenForeignClassMethods methods = bindBuiltInForeignClass(vm, module,
                                                            className);
  if (methods.allocate != NULL) return methods;

  if (bindClassFn != NULL)
  {
    return bindClassFn(vm, module, className);
  }

  return methods;
}

static void write(WrenVM* vm, const char* text)
{
  printf("%s", text);
}

static void reportError(WrenVM* vm, WrenErrorType type,
                        const char* module, int line, const char* message)
{
  switch (type)
  {
    case WREN_ERROR_COMPILE:
      fprintf(stderr, "[%s line %d] %s\n", module, line, message);
      break;
      
    case WREN_ERROR_RUNTIME:
      fprintf(stderr, "%s\n", message);
      break;
      
    case WREN_ERROR_STACK_TRACE:
      fprintf(stderr, "[%s line %d] in %s\n", module, line, message);
      break;
  }
}

static void initVM()
{
  WrenConfiguration config;
  wrenInitConfiguration(&config);

  config.bindForeignMethodFn = bindForeignMethod;
  config.bindForeignClassFn = bindForeignClass;
  config.resolveModuleFn = resolveModule;
  config.loadModuleFn = readModule;
  config.writeFn = write;
  config.errorFn = reportError;

  // Since we're running in a standalone process, be generous with memory.
  config.initialHeapSize = 1024 * 1024 * 100;
  vm = wrenNewVM(&config);

  // Initialize the event loop.
  loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
  uv_loop_init(loop);
}

static void freeVM()
{
  ioShutdown();
  schedulerShutdown();
  
  uv_loop_close(loop);
  free(loop);
  
  wrenFreeVM(vm);

  uv_tty_reset_mode();
}

void runFile(const char* path)
{
  char* source = readFile(path);
  if (source == NULL)
  {
    fprintf(stderr, "Could not find file \"%s\".\n", path);
    exit(66);
  }

  // Use the directory where the file is as the root to resolve imports
  // relative to.
  Path* directory = pathNew(path);
  pathDirName(directory);
  rootDirectory = pathToString(directory);
  pathFree(directory);
  
  Path* moduleName = pathNew(path);
  pathRemoveExtension(moduleName);

  initVM();

  WrenInterpretResult result = wrenInterpret(vm, moduleName->chars, source);

  if (afterLoadFn != NULL) afterLoadFn(vm);
  
  if (result == WREN_RESULT_SUCCESS)
  {
    uv_run(loop, UV_RUN_DEFAULT);
  }

  freeVM();

  free(source);
  free(rootDirectory);
  pathFree(moduleName);

  // Exit with an error code if the script failed.
  if (result == WREN_RESULT_COMPILE_ERROR) exit(65); // EX_DATAERR.
  if (result == WREN_RESULT_RUNTIME_ERROR) exit(70); // EX_SOFTWARE.
  
  if (defaultExitCode != 0) exit(defaultExitCode);
}

int runRepl()
{
  initVM();

  printf("\\\\/\"-\n");
  printf(" \\_/   wren v%s\n", WREN_VERSION_STRING);

  wrenInterpret(vm, "repl", "import \"repl\"\n");
  
  uv_run(loop, UV_RUN_DEFAULT);

  freeVM();

  return 0;
}

WrenVM* getVM()
{
  return vm;
}

uv_loop_t* getLoop()
{
  return loop;
}

void setExitCode(int exitCode)
{
  defaultExitCode = exitCode;
}

void setTestCallbacks(WrenBindForeignMethodFn bindMethod,
                      WrenBindForeignClassFn bindClass,
                      WrenForeignMethodFn afterLoad)
{
  bindMethodFn = bindMethod;
  bindClassFn = bindClass;
  afterLoadFn = afterLoad;
}

#include <stdio.h>
#include <string.h>

#include "uv.h"

#include "vm.h"
#include "os.h"
#include "wren.h"

#if __APPLE__
  #include "TargetConditionals.h"
#endif

int numArgs;
const char** args;

void osSetArguments(int argc, const char* argv[])
{
  numArgs = argc;
  args = argv;
}

void platformName(WrenVM* vm)
{
  wrenEnsureSlots(vm, 1);
  
  #ifdef _WIN32
    wrenSetSlotString(vm, 0, "Windows");
  #elif __APPLE__
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
      wrenSetSlotString(vm, 0, "iOS");
    #elif TARGET_OS_MAC
      wrenSetSlotString(vm, 0, "OS X");
    #else
      wrenSetSlotString(vm, 0, "Unknown");
    #endif
  #elif __linux__
    wrenSetSlotString(vm, 0, "Linux");
  #elif __unix__
    wrenSetSlotString(vm, 0, "Unix");
  #elif defined(_POSIX_VERSION)
    wrenSetSlotString(vm, 0, "POSIX");
  #else
    wrenSetSlotString(vm, 0, "Unknown");
  #endif
}

void platformIsPosix(WrenVM* vm)
{
  wrenEnsureSlots(vm, 1);
  
  #ifdef _WIN32
    wrenSetSlotBool(vm, 0, false);
  #elif __APPLE__
    wrenSetSlotBool(vm, 0, true);
  #elif __linux__
    wrenSetSlotBool(vm, 0, true);
  #elif __unix__
    wrenSetSlotBool(vm, 0, true);
  #elif defined(_POSIX_VERSION)
    wrenSetSlotBool(vm, 0, true);
  #else
    wrenSetSlotString(vm, 0, false);
  #endif
}

void processAllArguments(WrenVM* vm)
{
  wrenEnsureSlots(vm, 2);
  wrenSetSlotNewList(vm, 0);

  for (int i = 0; i < numArgs; i++)
  {
    wrenSetSlotString(vm, 1, args[i]);
    wrenInsertInList(vm, 0, -1, 1);
  }
}

void on_exit(uv_process_t *req, int64_t exit_status, int term_signal)
{
    printf("Process exited with status, signal %d\n", term_signal);
    uv_close((uv_handle_t*) req, NULL);
}

void spawnSubprocess(WrenVM* vm)
{
	// the number of args in the command
	int argsCount = wrenGetListCount(vm, 1);

	// an array of strings to store the args
	// makes mallocs in a loop, bad idea, but I don't know a better one
	// and this shouldn't be too hot/performant anyway
	char **args;
	args = malloc( ( argsCount  +  1 ) * sizeof(char*));

	//ensure we have enough slots to store the args:
	const int numberOfPreExitingSlots = 3;
	wrenEnsureSlots(vm, numberOfPreExitingSlots + argsCount);

	// get each arg out of the command array
	for(int i = 0; i < argsCount; i++){
		int length;

		//put the ith element into a slot to be read
		wrenGetListElement(vm, 1, i, i + numberOfPreExitingSlots);

		//read from the slot
		const char* wrenString = wrenGetSlotBytes(vm, i + numberOfPreExitingSlots, &length);

		//include the \0 character at the end
		length++;

		//move string into a locally managed store, REMEMBER TO FREE THIS!!!
		args[i] = malloc(length * sizeof(char));
		memcpy(args[i], wrenString, length);
	}
	//required by libuv
	args[argsCount] = NULL;


	uv_process_t child_req;
	uv_process_options_t options = {0};

	options.exit_cb = on_exit;
	options.file = args[0];
	options.args = args;

	int r;
	if ((r = uv_spawn(getLoop(), &child_req, &options))) {
		printf( "error: %s\n", uv_strerror(r));
	} else {
		wrenSetSlotDouble(vm, 0, (double)(child_req.pid));
	}

	//free args memory
	for(int i = 0; i < argsCount; i++){
		free(args[i]);
	}
	free(args);
}

void callSubprocess(WrenVM* vm)
{

}

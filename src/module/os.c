#include <stdio.h>
#include <string.h>
#include <inttypes.h>

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

//contains all information about a running sub-process
typedef struct subprocessBundle {
	bool running;
	uv_process_t childReq;
	uv_process_options_t options;
	uv_pipe_t out;
	uv_stdio_container_t stdio[2];
} subprocessBundle;

//number of concurrent subprocessess is currently hardcoded,
//wasn't sure if it was worth making this array dynamic
#define NUMBER_OF_SUB_PROCESSES 4

subprocessBundle subprocesses[NUMBER_OF_SUB_PROCESSES];

// handles for subprocess callback
WrenHandle* SubprocessClass;
WrenHandle* recieveStdOutMethod;
WrenHandle* recieveExitMethod;;

void onExit(uv_process_t *req, int64_t exitStatus, int termSignal)
{

	//first close the libuv handle for the subprocess
	uv_close((uv_handle_t*) req, NULL);

	//call the wren subprocess' onExitCB
	WrenVM* vm = getVM();

	unsigned int processesStillRunning = 0;
	for(int i = 0; i < NUMBER_OF_SUB_PROCESSES; i++){
		if(req == &subprocesses[i].childReq){
			if(!recieveExitMethod){
				recieveExitMethod = wrenMakeCallHandle( vm, "recieveExit(_,_)" );
			}

			wrenEnsureSlots(vm, 3);
			wrenSetSlotHandle(vm, 0, SubprocessClass);
			wrenSetSlotDouble(vm, 1, (double) subprocesses[i].childReq.pid);
			wrenSetSlotDouble(vm, 2, (double) exitStatus);

			wrenCall(vm, recieveExitMethod);

			subprocesses[i].running = false;
		}

		if(subprocesses[i].running){
			processesStillRunning++;
		}
	}

	//if there are no more processes running, clear our handles
	if(processesStillRunning == 0){
		wrenReleaseHandle(vm, SubprocessClass);
		wrenReleaseHandle(vm, recieveStdOutMethod);
		wrenReleaseHandle(vm, recieveExitMethod);
	}
}

// needed to alocate space as buffer grows, coppied from io.c
static void allocBuffer(uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf) 
{
	// TODO: Handle allocation failure.
	buf->base = (char*)malloc(suggestedSize);
	buf->len = suggestedSize;
}

void readOut(uv_stream_t* stream, ssize_t numRead, const uv_buf_t* buffer)
{
	if (numRead + 1 > (unsigned int)(buffer->len) || numRead == UV_EOF ){
		return;
	}

	buffer->base[numRead] = '\0'; // turn it into a cstring

	//call the wren subprocess onStdOutCB
	for(int i = 0; i < NUMBER_OF_SUB_PROCESSES; i++){
		if(stream == ( (uv_stream_t*) &subprocesses[i].out ) ){
			WrenVM* vm = getVM();

			wrenEnsureSlots(vm, 3);
			if(!SubprocessClass){
				wrenGetVariable(vm, "os", "Subprocess", 0);
				SubprocessClass = wrenGetSlotHandle(vm, 0);
			}

			if(!recieveStdOutMethod){
				recieveStdOutMethod = wrenMakeCallHandle(
						vm,
						"recieveStdOut_(_,_)"
						);
			}

			wrenSetSlotHandle(vm, 0, SubprocessClass);
			wrenSetSlotDouble(vm, 1, (double) subprocesses[i].childReq.pid);
			wrenSetSlotString(vm, 2, buffer->base);

			wrenCall(vm, recieveStdOutMethod);

			break;
		}
	}
}

void spawnSubprocess(WrenVM* vm)
{
	//find a subprocessBundle to use
	int bundleN = 0;
	for(int i = 0; i < NUMBER_OF_SUB_PROCESSES; i++){
		if(!subprocesses[i].running){
			subprocesses[i].running = true;
			bundleN = i;
			break;
		}
	}

	// the number of args in the command
	int argsCount = wrenGetListCount(vm, 1);

	// an array of strings to store the args
	// makes mallocs in a loop, bad idea, but I don't know a better one
	// and this shouldn't be too hot/performant anyway
	// we could do some calculations to store all the strings in one malloc,
	// TODO: discuss the pros/cons of performance vs readable code.
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

	//prepare the pipe for stdout
	uv_pipe_init(getLoop(), &subprocesses[bundleN].out, 0);

	//spawn options
	subprocesses[bundleN].options.stdio = subprocesses[bundleN].stdio;
	subprocesses[bundleN].stdio[0].flags = UV_IGNORE;
	subprocesses[bundleN].stdio[1].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
	subprocesses[bundleN].stdio[1].data.stream = (uv_stream_t*)&subprocesses[bundleN].out;
	subprocesses[bundleN].options.stdio_count = 2;

	subprocesses[bundleN].options.exit_cb = onExit;
	subprocesses[bundleN].options.file = args[0];
	subprocesses[bundleN].options.args = args;

	//start the SubProcess, prints to stderr if unsucsesful,
	//should probably bubble error up for wren to deal with
	int r;
	if ((r = uv_spawn(getLoop(), &(subprocesses[bundleN].childReq), &(subprocesses[bundleN].options)))) {
		printf( "error: %s\n", uv_strerror(r));
	}
	else {
		uv_read_start((uv_stream_t*)&subprocesses[bundleN].out, allocBuffer, readOut);
		//return the Subprocess PID, which will be used as its identifier for passing 
		//information in and out of wren,
		//TODO: is a PID the right fit for this use, should we generate a seperate
		//random ID?
		wrenSetSlotDouble(vm, 0, (double)(subprocesses[bundleN].childReq.pid));
	}

	//free args memory
	for(int i = 0; i < argsCount; i++){
		free(args[i]);
	}
	free(args);
}

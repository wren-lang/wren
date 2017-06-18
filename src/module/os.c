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


/* subprocess callbacks and variables */
void on_exit(uv_process_t *req, int64_t exit_status, int term_signal)
{
	uv_close((uv_handle_t*) req, NULL);
}

static void alloc_buffer(uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf) 
{
  // TODO: Handle allocation failure.
  buf->base = (char*)malloc(suggestedSize);
  buf->len = suggestedSize;
}

typedef struct subprocessBundle {
	bool running;
	uv_process_t child_req;
	uv_process_options_t options;
	uv_pipe_t out;
	uv_stdio_container_t stdio[2];
} subprocessBundle;

#define NUMBER_OF_SUB_PROCESSES 4

subprocessBundle subprocesses[NUMBER_OF_SUB_PROCESSES];

WrenHandle* SubprocessClass;
WrenHandle* recieveStdOutMethod;

void read_out(uv_stream_t* stream, ssize_t numRead, const uv_buf_t* buffer)
{
    if (numRead + 1 > (unsigned int)(buffer->len) || numRead == UV_EOF ){
		return;
	}

    buffer->base[numRead] = '\0'; // turn it into a cstring

	for(int i = 0; i < NUMBER_OF_SUB_PROCESSES; i++){
		if(stream == ( (uv_stream_t*) &subprocesses[i].out ) ){
			/*fprintf(stderr, "got stdout for %d\n", subprocesses[i].child_req.pid);*/
			/*printf("read: |%s|", buffer->base);*/

			//call wren::Subprocess::recieveStdOut(_, _)
			//
			// Load the class into slot 0.
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
			wrenSetSlotDouble(vm, 1, (double) subprocesses[i].child_req.pid);
			wrenSetSlotString(vm, 2, buffer->base);

			wrenCall(vm, recieveStdOutMethod);

			break;
		}
	}
}

void spawnSubprocess(WrenVM* vm)
{

	//find a subprocessBundle to use
	//limited to a constant number of subprocess ATM
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
	// 
	// we could do some calculations to store all the strings in one malloc,
	// discuss the pros/cons of performance vs readable code.
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

	subprocesses[bundleN].options.exit_cb = on_exit;
	subprocesses[bundleN].options.file = args[0];
	subprocesses[bundleN].options.args = args;

	int r;
	if ((r = uv_spawn(getLoop(), &(subprocesses[bundleN].child_req), &(subprocesses[bundleN].options)))) {
		printf( "error: %s\n", uv_strerror(r));
	}
	else {
		uv_read_start((uv_stream_t*)&subprocesses[bundleN].out, alloc_buffer, read_out);
		wrenSetSlotDouble(vm, 0, (double)(subprocesses[bundleN].child_req.pid));
	}

	//free args memory
	for(int i = 0; i < argsCount; i++){
		free(args[i]);
	}
	free(args);
}

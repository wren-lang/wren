#ifndef wren_debugger_h
#define wren_debugger_h

#include "wren.h"
#include "wren_value.h"
#include "wren_common.h"
#include "wren_compiler.h"

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

#if WREN_DEBUGGER

#define WREN_MAX_BREAKPOINTS 32
#define MAX_NAME_LEN 256

typedef struct {
    int id;
    int line;
    int stopped_in_frame; //the idx of the callframe during which we last stopped on this breakpoint (or -1 if we have left that callframe)
    char module[MAX_NAME_LEN]; //This should probably be dynamic to avoid the length restriction, but for it's easier this way
} Breakpoint;

typedef enum {
    //Execution is halted until we get a command to advance
    WREN_DEBUGGER_STATE_HALTING,

    //Exeuction is running
    WREN_DEBUGGER_STATE_RUNNING,

    //Exeuction is running until we've stepped over the whole line we're on
    WREN_DEBUGGER_STATE_STEPPING_OVER,

    //Execution is running until we've entered the next function on this line, or moved on to the next line
    WREN_DEBUGGER_STATE_STEPPING_INTO,

    //Execution is running until we arrive back at the calling function
    WREN_DEBUGGER_STATE_STEPPING_OUT
} DebuggerState;

typedef enum {
  // Don't do anything
  WREN_DEBUGGER_CMD_NONE,
  
  // Continue execution until the next breakpoint
  WREN_DEBUGGER_CMD_CONTINUE,

  //Step over current line
  WREN_DEBUGGER_CMD_STEP_OVER,

  //Step into next function on this line
  WREN_DEBUGGER_CMD_STEP_INTO,

  //Step out of the current function (ignored if triggered during the root function)
  WREN_DEBUGGER_CMD_STEP_OUT

} DebuggerCmd;

typedef enum {
    //Didn't stop
  WREN_DEBUGGER_STOP_DIDNT,

    //Stopped due to a fiber switch
  WREN_DEBUGGER_STOP_SWITCH,

    //Stopped due to stepping in/out/over
  WREN_DEBUGGER_STOP_STEP,

    //Stopped due to a breakpoint
  WREN_DEBUGGER_STOP_BREAKPOINT

} DebuggerStopReason;

typedef enum {

  WREN_DEBUGGER_EVENT_CREATED_BREAKPOINT,

  WREN_DEBUGGER_EVENT_STOPPED,

  WREN_DEBUGGER_EVENT_STACK,

  WREN_DEBUGGER_EVENT_SOURCE,

  WREN_DEBUGGER_EVENT_VARS

} DebuggerEventType;

typedef struct {
    int listen_sock;
    int comm_sock;

    Breakpoint breakpoints[WREN_MAX_BREAKPOINTS]; //:todo: this should be a module name -> lines map
    int num_breakpoints;
    int next_id;

    DebuggerState state;

      //:todo: weird way to identify our fiber, but we never dereference it so should be fine for GC.
    ObjFiber* last_fiber; 

    int last_line;
    int last_frame;

        //Info about the last place we were before stepping over a line
    int last_step_line;
    int last_step_frame;

    int target_step_out_frame;
} WrenDebugger;

void wrenInitDebugger(WrenVM* vm);
void wrenFreeDebugger(WrenVM* vm);
void wrenResetDebugger(WrenVM* vm);

void wrenRunDebugger(WrenVM* vm, ObjFn* fn, int i);

int debuggerAddBreakpoint(WrenDebugger* debugger, const char* module, int line);
bool debuggerRemoveBreakpoint(WrenDebugger* debugger, const char* module, int line);
void debuggerSendStack(WrenVM* vm, const char* msg, int ip);
void debuggerSendSource(WrenVM* vm, const char* msg);
void debuggerSendVar(WrenVM* vm, const char* msg);
void debuggerSendModuleInfo(WrenVM* vm, int stack_idx);
void debuggerSendFunctionInfo(WrenVM* vm, int stack_idx);

void debuggerSendEvent(int socket, DebuggerEventType type, const char* data, int length, bool finish);

DebuggerStopReason debuggerShouldBreak(WrenDebugger* debugger, const ObjFiber* fiber, int frame, const char* module, int line);

bool debuggerPollInput(WrenVM* vm);
void wrenDebuggerPollConfigCmds(WrenVM* vm);
DebuggerCmd debuggerGetCmd(WrenVM* vm, int ip);
bool debuggerProcessConfigCmds(WrenVM* vm, const char* msg);
DebuggerCmd debuggerProcessStoppedCmds(WrenVM* vm, const char* msg, int ip);

#endif //WREN_DEBUGGER

#endif
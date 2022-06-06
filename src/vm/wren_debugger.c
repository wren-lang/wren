#include "wren_debugger.h"

#if WREN_DEBUGGER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
    
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h> 
#include <netdb.h>
#include <sys/socket.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include "wren_vm.h"
#include "wren_debug.h"

#if WREN_OPT_META
  #include "wren_opt_meta.h"
#endif
#if WREN_OPT_RANDOM
  #include "wren_opt_random.h"
#endif

DEFINE_BUFFER(SymbolTable, SymbolTable);
DEFINE_BUFFER(IntBuffer, IntBuffer);

#define EVENT_BUFFER_SIZE 4096

static char msg[256];
static char* msg_cursor;

void wrenInitDebugger(WrenVM* vm) {

    vm->debugger.num_breakpoints = 0;
    vm->debugger.next_id = 0;

    vm->debugger.listen_sock = -1;
    vm->debugger.comm_sock = -1;

    if(vm->config.enableDebugger) {

        int status;
        struct addrinfo hints;
        struct addrinfo *servinfo;      // will point to the results

        memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_INET;       // use IPv4
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
        hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

        status = getaddrinfo(NULL, vm->config.debuggerPort, &hints, &servinfo);

        if (status != 0) {
          #ifdef _WIN32
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerrorA(status));
          #else
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
          #endif
          // exit(1);
        } else {
                // servinfo now points to a linked list of 1 or more struct addrinfos
            vm->debugger.listen_sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
            
            #ifdef _WIN32
                unsigned long non_blocking = 1;
                ioctlsocket(vm->debugger.listen_sock, FIONBIO, &non_blocking);
            #else
                fcntl(vm->debugger.listen_sock, F_SETFL, O_NONBLOCK);
            #endif

            bind(vm->debugger.listen_sock, servinfo->ai_addr, servinfo->ai_addrlen);

            freeaddrinfo(servinfo); // free the linked-list

            listen(vm->debugger.listen_sock, 10);
        }

        // printf("luxe / wren / debugger / started\n"); //:todo: these won't land in log
    } //if enableDebugger

    msg_cursor = msg;
    memset(msg, 0, sizeof msg);

    wrenResetDebugger(vm);
}

void wrenFreeDebugger(WrenVM* vm) {
    #ifdef _WIN32
        closesocket(vm->debugger.comm_sock);
        closesocket(vm->debugger.listen_sock);
    #else
        close(vm->debugger.comm_sock);
        close(vm->debugger.listen_sock);
    #endif

    // printf("luxe / wren / debugger / shutdown\n"); //:todo: these won't land in log
}

void wrenResetDebugger(WrenVM* vm) {
    vm->debugger.state = WREN_DEBUGGER_STATE_RUNNING;
    vm->debugger.last_line = -1;
    vm->debugger.last_frame = -1;

    vm->debugger.last_step_line = -1;
    vm->debugger.last_step_frame = -1;

    vm->debugger.target_step_out_frame = -1;

    vm->debugger.last_fiber = vm->fiber;
}

void wrenRunDebugger(WrenVM* vm, ObjFn* fn, int i) {
    if(fn->module == NULL || fn->module->name == NULL) {
        return; //internal functions don't have modules assigned, for now we just don't allow stopping here
    }

    int line = fn->debug->sourceLines.data[i];
    int frameIdx = vm->fiber->numFrames - 1;
    const char* module = fn->module->name->value;

    DebuggerStopReason reason = debuggerShouldBreak(&vm->debugger, vm->fiber, frameIdx, module, line);
    if(reason != WREN_DEBUGGER_STOP_DIDNT) {
        vm->debugger.state = WREN_DEBUGGER_STATE_HALTING;

        char out[64];
        int out_len = sprintf(out, "stopped %d\n", reason);
        debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_STOPPED, out, out_len, true);
    }

    if(frameIdx != vm->debugger.last_frame) {
        for(int i=0; i<vm->debugger.num_breakpoints; i++) {
            if(vm->debugger.breakpoints[i].stopped_in_frame > frameIdx) { //if we stepped out of a frame, reset all breakpoints in deeper frames we may have triggered
                vm->debugger.breakpoints[i].stopped_in_frame = -1;
            }
        }
    }

        //treat fiber switches as a complete callframe reset for now
    if(vm->fiber != vm->debugger.last_fiber) {
        for(int i=0; i<vm->debugger.num_breakpoints; i++) {
            vm->debugger.breakpoints[i].stopped_in_frame = -1;
        }
    }

    vm->debugger.last_fiber = vm->fiber;
    vm->debugger.last_frame = frameIdx;
    vm->debugger.last_line = line;

    while(vm->debugger.state == WREN_DEBUGGER_STATE_HALTING) {
        #ifdef _WIN32
          Sleep(100);
        #else
          usleep(100000); //0.1 seconds
        #endif

        DebuggerCmd cmd = debuggerGetCmd(vm, i);

        switch(cmd) {
            case WREN_DEBUGGER_CMD_CONTINUE:
                vm->debugger.state = WREN_DEBUGGER_STATE_RUNNING;
                break;

            case WREN_DEBUGGER_CMD_STEP_OVER:
                vm->debugger.last_step_line = line;
                vm->debugger.last_step_frame = frameIdx;
                vm->debugger.state = WREN_DEBUGGER_STATE_STEPPING_OVER;
                break;

            case WREN_DEBUGGER_CMD_STEP_INTO:
                vm->debugger.state = WREN_DEBUGGER_STATE_STEPPING_INTO;
                break;

            case WREN_DEBUGGER_CMD_STEP_OUT:
                if(frameIdx == 0) break; //Ignore if there's nowhere to step out to
                vm->debugger.state = WREN_DEBUGGER_STATE_STEPPING_OUT;
                vm->debugger.target_step_out_frame = frameIdx - 1;
                break;

            case WREN_DEBUGGER_CMD_NONE:
                break;
        }
    }
}


    //next: add/remove with module, update command parsing to also grab module string
int debuggerAddBreakpoint(WrenDebugger* debugger, const char* module, int line) {
    for(int i=0; i < debugger->num_breakpoints; ++i) {
        Breakpoint* breakpoint = &debugger->breakpoints[i];
        if(breakpoint->line == line && (strcmp(breakpoint->module, module) == 0)) return -1;
    }

    if(debugger->num_breakpoints < WREN_MAX_BREAKPOINTS) {
        debugger->breakpoints[debugger->num_breakpoints].line = line;
        strcpy(debugger->breakpoints[debugger->num_breakpoints].module, module);
        debugger->breakpoints[debugger->num_breakpoints].id = debugger->next_id;
        debugger->breakpoints[debugger->num_breakpoints].stopped_in_frame = -1;
        debugger->next_id++;
        debugger->num_breakpoints++;
        return debugger->next_id - 1;
    } else {
        printf("Debugger / add brekapoint / Reached the maximum number breakpoints allowed (%d)\n", WREN_MAX_BREAKPOINTS);
        return -1;
    }
}

void debuggerSendStack(WrenVM* vm, const char* msg, int ip) {

	char root_path[4096];    
    int status = sscanf(msg, "stack \"%4096[^\"]\"", root_path); 
    if(status != 1) {
        
    }

    ObjFiber* fiber = vm->fiber;

    int num_frames = fiber->numFrames;

    char out[6144]; //typical max path length * 1.5 should be a reasonable default

    for(int i=0; i<num_frames; i++) {
        CallFrame* frame = &fiber->frames[i];
        ObjFn* fn = frame->closure->fn;

        //:todo: this follows the behaviour the of wrenDebugPrintStackTrace, see later if this makes sense for step through debugging

            //skip over stub functions for calling methods from the C API
        if(fn->module == NULL) continue;

            //skip over core modules that have no name
        if(fn->module->name == NULL) continue;

        char* module = fn->module->name->value;
        char* fnName = fn->debug->name;
            // -1 because IP has advanced past the instruction that it just executed.
        int line = fn->debug->sourceLines.data[frame->ip - fn->code.data - 1];
        if(i == num_frames - 1) {
            line = fn->debug->sourceLines.data[ip]; //the last frame doesn't have its IP updated in the interpreter loop, so apply the one we know from being called during the loop
        }
        
        //:todo: if modulePathFn is not specified this is blow up city
        const char* path = vm->config.modulePathFn(vm, module, root_path);
        const char* print_path = (path == NULL) ? "" : path;

            //:todo: module could contain '|' and mess up the pattern
        int out_len = sprintf(out, "%d|%s|%s|%s|%d\n", i, module, print_path, fnName, line);
        debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_STACK, out, out_len, false);

        free((void *)path); //:todo: redo memory management to not require malloc/free
    }

    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_STACK, NULL, 0, true);
}

void debuggerSendSource(WrenVM* vm, const char* msg) {
    char module[128];
    int status = sscanf(msg, "source \"%127[^\"]\"", module);

    const char* source = NULL;
    int source_len = 0;

    if(status == 1) {

        // Let the host try to provide the module.
        if (vm->config.loadModuleFn != NULL)
        {
            //:todo: Update comments in config, this causes the load fn to be called more than once
            WrenLoadModuleResult res = vm->config.loadModuleFn(vm, module);
            source = res.source;
        }

        // If the host didn't provide it, see if it's a built in optional module.
        if (source == NULL)
        {
            #if WREN_OPT_META
                if (strncmp(module, "meta", 4) == 0) source = wrenMetaSource();
            #endif
            #if WREN_OPT_RANDOM
                if (strncmp(module, "random", 6) == 0) source = wrenRandomSource();
            #endif
        }

        if(source != NULL) source_len = strlen(source);
    }

        //for source_len == 0 this will just send an event message with empty payload, to ensure there is always some response to the source request
    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_SOURCE, source, source_len, true);
}

void debuggerDumpObject(Obj* obj) {
    switch (obj->type)
    {
        case OBJ_CLASS:
            printf("Class %s", ((ObjClass*)obj)->name->value);
            break;
        case OBJ_CLOSURE: 
            printf("Function %s", ((ObjClosure*)obj)->fn->debug->name);
            break;
        case OBJ_FIBER: printf("Fiber %p", obj); break;
        case OBJ_FN: printf("[fn %p]", obj); break;
        case OBJ_FOREIGN: printf("[foreign %p]", obj); break;
        case OBJ_INSTANCE: 
            printf("Instance of class %s", obj->classObj->name->value);
            break;
        case OBJ_LIST: printf("[list %p]", obj); break;
        case OBJ_MAP: printf("[map %p]", obj); break;
        case OBJ_MODULE: printf("[module %p]", obj); break;
        case OBJ_RANGE: printf("[range %p]", obj); break;
        case OBJ_STRING: printf("%s", ((ObjString*)obj)->value); break;
        case OBJ_UPVALUE: printf("[upvalue %p]", obj); break;
        default: printf("[unknown object %d]", obj->type); break;
    }
}

void debuggerDumpValue(Value value) {
#if WREN_NAN_TAGGING
    if (IS_NUM(value))
    {
        printf("%.14g", AS_NUM(value));
    }
    else if (IS_OBJ(value))
    {
        debuggerDumpObject(AS_OBJ(value));
    }
    else
    {
        switch (GET_TAG(value))
        {
            case TAG_FALSE:     printf("false"); break;
            case TAG_NAN:       printf("NaN"); break;
            case TAG_NULL:      printf("null"); break;
            case TAG_TRUE:      printf("true"); break;
            case TAG_UNDEFINED: UNREACHABLE();
        }
    }
#else
    switch (value.type)
    {
        case VAL_FALSE:     printf("false"); break;
        case VAL_NULL:      printf("null"); break;
        case VAL_NUM:       printf("%.14g", AS_NUM(value)); break;
        case VAL_TRUE:      printf("true"); break;
        case VAL_OBJ:       debuggerDumpObject(AS_OBJ(value)); break;
        case VAL_UNDEFINED: UNREACHABLE();
    }
#endif
}

void debuggerSendVar(WrenVM* vm, const char* msg) {
    char var[MAX_VARIABLE_NAME];
    //:todo: 64 in format string should be MAX_VARIABLE_NAME, but it requires some additional macros
    int status = sscanf(msg, "var \"%64[^\"]\"", var); 

    if(status != 1) {
        return;
    }

    int var_name_len = strlen(var);
    if(var_name_len == 0) {
        printf("Variable name can't be empty\n");
        return;
    }

    Value val;
    bool found = false;

    CallFrame* frame = &vm->fiber->frames[vm->fiber->numFrames - 1];
    FnDebug* debug = frame->closure->fn->debug;

    int local = wrenSymbolTableFind(&debug->locals.locals, var, var_name_len);
    if(local != -1) { // local or upvalue

        int startLine = debug->locals.startLines.data[local];
        int endLine = debug->locals.endLines.data[local];

        if(startLine < vm->debugger.last_line && vm->debugger.last_line <= endLine) {
            int slot = debug->locals.localIndexes.data[local];
            int isUpvalue = debug->locals.isUpvalue.data[local];

            if(isUpvalue) {
                ObjUpvalue** upvalues = frame->closure->upvalues;
                val = *upvalues[slot]->value;
                found = true;
            } else {
                val = frame->stackStart[slot];
                found = true;
            }
        }
    } else { // field or module
            // It's a field if it starts with exactly one _
        if(var[0] == '_' && (var_name_len == 1 || var[1] != '_')) {
            Value receiver = frame->stackStart[0];

            if(IS_INSTANCE(receiver)) {
                ObjInstance* instance = AS_INSTANCE(receiver);
                ObjClass* classObj = instance->obj.classObj;
                ObjModule* module = classObj->module;

                int moduleClassIdx = wrenSymbolTableFind(&module->classDebug.classIndices, classObj->name->value, classObj->name->length);

                if(moduleClassIdx != -1) {
                    SymbolTable* fields = &module->classDebug.fieldIndices.data[moduleClassIdx];
                    IntBuffer* slots = &module->classDebug.fieldSlots.data[moduleClassIdx];

                    int fieldIdx = wrenSymbolTableFind(fields, var, var_name_len);
                    if(fieldIdx != -1) {
                        int slot = slots->data[fieldIdx];
                        val = instance->fields[slot];
                        found = true;
                    }
                }
            }
        } else { // module variable
            ObjModule* module = frame->closure->fn->module;
            int slot = wrenSymbolTableFind(&module->variableNames, var, var_name_len);
            if(slot != -1) {
                val = module->variables.data[slot];
                found = true;
            }
        }
    }

    if(found) {
        debuggerDumpValue(val);
        printf("\n");
    } else {
        printf("Variable is not defined here.\n");
    }
}

int sprintObj(char* buffer, Obj* obj) {
    switch (obj->type)
    {
        case OBJ_CLASS:
            return sprintf(buffer, "Class %s", ((ObjClass*)obj)->name->value);
        case OBJ_CLOSURE: 
            return sprintf(buffer, "Function %s", ((ObjClosure*)obj)->fn->debug->name);
        case OBJ_FIBER: return sprintf(buffer, "Fiber %p", obj);
        case OBJ_FN: return sprintf(buffer, "[fn %p]", obj);
        case OBJ_FOREIGN: return sprintf(buffer, "[foreign %p]", obj);
        case OBJ_INSTANCE: 
            return sprintf(buffer, "Instance of class %s", obj->classObj->name->value);
        case OBJ_LIST: return sprintf(buffer, "[list %p]", obj);
        case OBJ_MAP: return sprintf(buffer, "[map %p]", obj);
        case OBJ_MODULE: return sprintf(buffer, "[module %p]", obj);
        case OBJ_RANGE: return sprintf(buffer, "[range %p]", obj);
        case OBJ_STRING: return sprintf(buffer, "%s", ((ObjString*)obj)->value);
        case OBJ_UPVALUE: return sprintf(buffer, "[upvalue %p]", obj);
        default: return sprintf(buffer, "[unknown object %d]", obj->type);
    }
}

const char * const objTypeNames[] = { 
    "Class", 
    "Closure", 
    "Fiber", 
    "Fn",
    "Foreign",
    "Instance",
    "List",
    "Map",
    "Module",
    "Range",
    "String",
    "Upvalue"
};

const char* valueTypeName(const Value value) {
    #if WREN_NAN_TAGGING
        if(IS_NUM(value)) return "Num";
        if(IS_OBJ(value)) {
            return objTypeNames[AS_OBJ(value)->type];
        }
        //not num or obj, so a singleton as specified by tags
        switch(GET_TAG(value)) {
            case TAG_NAN: return "NaN"; //:todo: Should this count as Num?
            case TAG_NULL: return "Null";
            case TAG_FALSE: //intentional fallthrough
            case TAG_TRUE: return "Bool";
            case TAG_UNDEFINED: UNREACHABLE();
        }
    #else
        switch (value.type)
        {
            case VAL_FALSE: //intentional fallthrough
            case VAL_TRUE: return "Bool";
            case VAL_NULL: return "Null";
            case VAL_NUM: return "Num";
            
            case VAL_OBJ: return objTypeNames[AS_OBJ(value)->type];
            case VAL_UNDEFINED: UNREACHABLE();
        }
    #endif

    return NULL; //shouldn't happen due to unreachables above
}

    //:todo: find a better solution for this. Static here since allocating this 
    //in an if statement and pointing to it will free the buffer when exiting the if
    //so permanent storage is needed
static char printBuffer[310];//max value of num is ~1.8 x 10^308 (which is the max int in a 64bit double)
                             // so 310 chars should be enough to hold the maximum length printed value with \0 and a char extra to be safe
void internalSendVar(WrenVM* vm, const ObjString* name, const Value val) {
    char* valuePrinted;
    int printedLen = 0;

    #if WREN_NAN_TAGGING
        if (IS_OBJ(val) && AS_OBJ(val)->type == OBJ_STRING) {
            valuePrinted = AS_STRING(val)->value;
            printedLen = AS_STRING(val)->length;
        } else {
            valuePrinted = printBuffer;
            if (IS_NUM(val)) {
                printedLen = sprintf(valuePrinted, "%.14g", AS_NUM(val));
            } else if (IS_OBJ(val)) {
                printedLen = sprintObj(valuePrinted, AS_OBJ(val));
            } else {
                switch (GET_TAG(val))
                {
                    case TAG_FALSE:     printedLen = sprintf(valuePrinted, "false"); break;
                    case TAG_NAN:       printedLen = sprintf(valuePrinted, "NaN"); break;
                    case TAG_NULL:      printedLen = sprintf(valuePrinted, "null"); break;
                    case TAG_TRUE:      printedLen = sprintf(valuePrinted, "true"); break;
                    case TAG_UNDEFINED: UNREACHABLE();
                }
            }
        }
    #else
        if(val.type == VAL_OBJ && AS_OBJ(val)->type == OBJ_STRING) {
            valuePrinted = AS_STRING(val)->value;
            printedLen = AS_STRING(val)->length;
        } else {
            valuePrinted = printBuffer;

            switch (val.type)
            {
                case VAL_FALSE:     printedLen = sprintf(valuePrinted, "false"); break;
                case VAL_NULL:      printedLen = sprintf(valuePrinted, "null"); break;
                case VAL_NUM:       printedLen = sprintf(valuePrinted, "%.14g", AS_NUM(value)); break;
                case VAL_TRUE:      printedLen = sprintf(valuePrinted, "true"); break;
                case VAL_OBJ:       printedLen = sprintObj(valuePrinted, AS_OBJ(value)); break;
                case VAL_UNDEFINED: UNREACHABLE();
            }
        }
    #endif

    //send the pattern of "varname|printedLen|printedValue" where printedLen gives the length of printedValue in bytes, to avoid having to escape strings

    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_VARS, name->value, name->length, false);
    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_VARS, "|", 1, false);
    const char* typeName = valueTypeName(val);
    int typeNameLen = strlen(typeName);
    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_VARS, typeName, typeNameLen, false);
    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_VARS, "|", 1, false);    
    char lenBuffer[11]; //Positive 32bit int can bet at most 10 digits printed, + \0
    int lenBufferLen = sprintf(lenBuffer, "%d", printedLen); //:todo: oh god better naming please

    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_VARS, lenBuffer, lenBufferLen, false);
    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_VARS, "|", 1, false);

    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_VARS, valuePrinted, printedLen, false);
    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_VARS, "\n", 1, false);
}

    //:todo: naming
    //gets the stack frame corresponding to the index into the stackframe we'd print out
    //Crucially, printed stackframes skip frames from internal code (core module, foreign functions),
    //so we have to skip these when counting
CallFrame* getStackIdxFrame(WrenVM* vm, int stack_idx) {
    ObjFiber* fiber = vm->fiber;

    int running_idx = 0;
    for(int i=0; i<fiber->numFrames; i++) {
        CallFrame* frame = &fiber->frames[i];
        ObjFn* fn = frame->closure->fn;

        //:todo: this follows the behaviour the of wrenDebugPrintStackTrace, see later if this makes sense for step through debugging

            //skip over stub functions for calling methods from the C API
        if(fn->module == NULL) continue;

            //skip over core modules that have no name
        if(fn->module->name == NULL) continue;

        if(running_idx == stack_idx) {
            return frame;
        }

        running_idx++;
    }

    return NULL;
}

void debuggerSendModuleInfo(WrenVM* vm, int stack_idx) {
    CallFrame* frame = getStackIdxFrame(vm, stack_idx);
    if(!frame) return;
    ObjModule* module = frame->closure->fn->module;

    for(int i=0; i<module->variableNames.count; i++) {
        ObjString* name = module->variableNames.data[i];
        Value val = module->variables.data[i];

        internalSendVar(vm, name, val);
    }

    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_VARS, NULL, 0, true);
}

void debuggerSendFunctionInfo(WrenVM* vm, int stack_idx) {
    CallFrame* frame = getStackIdxFrame(vm, stack_idx);
    if(!frame) return;
    FnDebug* debug = frame->closure->fn->debug;

    for(int i=0; i<debug->locals.localIndexes.count; i++) {
        int startLine = debug->locals.startLines.data[i];
        int endLine = debug->locals.endLines.data[i];

        if(startLine < vm->debugger.last_line && vm->debugger.last_line <= endLine) {
            int slot = debug->locals.localIndexes.data[i];
            int isUpvalue = debug->locals.isUpvalue.data[i];

            Value val;

            if(isUpvalue) {
                ObjUpvalue** upvalues = frame->closure->upvalues;
                val = *upvalues[slot]->value;
            } else {
                val = frame->stackStart[slot];
            }

            ObjString* name = debug->locals.locals.data[i];
            internalSendVar(vm, name, val);
        }
    }

    Value receiver = frame->stackStart[0];

    if(IS_INSTANCE(receiver)) {
        ObjInstance* instance = AS_INSTANCE(receiver);
        ObjClass* classObj = instance->obj.classObj;
        ObjModule* module = classObj->module;

        int moduleClassIdx = wrenSymbolTableFind(&module->classDebug.classIndices, classObj->name->value, classObj->name->length);

        if(moduleClassIdx != -1) {
            SymbolTable* fields = &module->classDebug.fieldIndices.data[moduleClassIdx];
            IntBuffer* slots = &module->classDebug.fieldSlots.data[moduleClassIdx];

            for(int i=0; i<fields->count; i++) {
                ObjString* name = fields->data[i];
                int slot = slots->data[i];
                Value val = instance->fields[slot];

                internalSendVar(vm, name, val);
            }
        }
    }

    debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_VARS, NULL, 0, true);
}

static char event_buffer[EVENT_BUFFER_SIZE];
static int event_cursor = 0;

    //copies as much data as possible to the buffer, and then dispatches an event message if the buffer is full or finish is true
    //returns the number of characters consumed from data
static int internalSendEvent(int socket, DebuggerEventType type, const char* data, int length, bool finish) {
    int copy_len = 0;
    if(length != 0) {
        copy_len = length < (EVENT_BUFFER_SIZE - event_cursor) ? length : (EVENT_BUFFER_SIZE - event_cursor);
        memcpy(event_buffer + event_cursor, data, copy_len);

        event_cursor += copy_len;
    }

    if(event_cursor == EVENT_BUFFER_SIZE || finish) {
        char out[64];
        int out_len = sprintf(out, "event: %d\n", type);
        send(socket, out, out_len, 0);

        out_len = sprintf(out, "length: %d\n", event_cursor);
        send(socket, out, out_len, 0);

        out_len = sprintf(out, "final: %d\n", (event_cursor == EVENT_BUFFER_SIZE) ? 0 : 1);
        send(socket, out, out_len, 0);

        send(socket, event_buffer, event_cursor, 0);

        event_cursor = 0;
    }

    return copy_len;
}

    //build up an event of the given type, with length characters from data copied to the payload of the event
    //multiple calls in sequence will append additional data to the payload
    //if the payload reaches EVENT_BUFFER_SIZE, an event message will be sent, and the remaining data will be copied to the next message's payload
    //setting finish to true ensures that any remaining data is sent, or an empty message if no data was specified since the last call with finish == true
void debuggerSendEvent(int socket, DebuggerEventType type, const char* data, int length, bool finish) {
    int data_offset = 0;
        //do-while to ensure at least one call even if length is 0. This enables "finalizing" a send without any additional data
    do {
        int consumed = internalSendEvent(socket, type, data + data_offset, length - data_offset, finish);
        data_offset += consumed;
    } while(data_offset < length);
}

bool debuggerRemoveBreakpoint(WrenDebugger* debugger, const char* module, int line) {
    for (int i = 0; i < debugger->num_breakpoints; ++i) {
        Breakpoint* breakpoint = &debugger->breakpoints[i];
        if(breakpoint->line == line && (strcmp(breakpoint->module, module) == 0)) {
            breakpoint->line = debugger->breakpoints[debugger->num_breakpoints - 1].line;
            strcpy(breakpoint->module, debugger->breakpoints[debugger->num_breakpoints - 1].module);
            debugger->num_breakpoints--;
            return true;
        }
    }

    return false;
}

DebuggerStopReason debuggerShouldBreak(WrenDebugger* debugger, const ObjFiber* fiber, int frame, const char* module, int line) {

        //fiber switching breaks assumptions on stepping over/out, so for now all switches cause a break
        //For stepping into, we want to break upon switch anyway, so we can just always break while not running
    if(debugger->state != WREN_DEBUGGER_STATE_RUNNING) {
        if(fiber != debugger->last_fiber) {
            //:todo: clear stepping values maybe? The frame idx was for the old fiber, it's not valid anymore
            return WREN_DEBUGGER_STOP_SWITCH;
        }
    }

    bool step_line_diff = line != debugger->last_step_line;
    bool step_frame_diff = frame != debugger->last_step_frame;

    if(debugger->state == WREN_DEBUGGER_STATE_STEPPING_OVER) {
        if(step_line_diff && !step_frame_diff) {
            return WREN_DEBUGGER_STOP_STEP;
        } else if(!step_line_diff && !step_frame_diff) {
                //definitely step over the line we started from
                //the line might've had a breakpoint on it, so we don't want the later code to make us break there again
            return WREN_DEBUGGER_STOP_DIDNT;
        }
    }

    bool line_diff = line != debugger->last_line;
    bool frame_diff = frame != debugger->last_frame;

    if(debugger->state == WREN_DEBUGGER_STATE_STEPPING_INTO) {
        if(line_diff || frame_diff) {
            return WREN_DEBUGGER_STOP_STEP;
        }
    }

    if(debugger->state == WREN_DEBUGGER_STATE_STEPPING_OUT) {
        if(frame == debugger->target_step_out_frame) return WREN_DEBUGGER_STOP_STEP;
    }

    if(line_diff || frame_diff) {
        for(int i=0; i<debugger->num_breakpoints; i++) {
            if(debugger->breakpoints[i].stopped_in_frame < frame) { //we don't want to re-hit a breakpoint if we already triggered it at the same callframe level
                if(debugger->breakpoints[i].line == line && (strcmp(debugger->breakpoints[i].module, module) == 0)) {
                    debugger->breakpoints[i].stopped_in_frame = frame;
                    return WREN_DEBUGGER_STOP_BREAKPOINT;
                }
            }
        }
    }
    
    return WREN_DEBUGGER_STOP_DIDNT;
}

bool debuggerPollInput(WrenVM* vm) {
    if(vm->debugger.comm_sock == -1) {
        vm->debugger.comm_sock = accept(vm->debugger.listen_sock, (struct sockaddr*) NULL, NULL);
        if(vm->debugger.comm_sock > 0) {
            printf("luxe / wren / debugger / connection opened\n");
            #ifdef _WIN32
                unsigned long non_blocking = 1;
                ioctlsocket(vm->debugger.comm_sock, FIONBIO, &non_blocking);
            #else
                fcntl(vm->debugger.comm_sock, F_SETFL, O_NONBLOCK);
            #endif
        }
    } else {
            //if we're not at the end of the string, advance to the next line
        if(*msg_cursor != '\0') {
            while(*msg_cursor != '\n') {
                msg_cursor++;
            }
            msg_cursor++; //step past the \n
        }

            //if it wasn't the last line, we got more input to process
        if(*msg_cursor != '\0') {
            return true;
        }

            //otherwise reset the cursor and read new data
        msg_cursor = msg;
        memset(msg, 0, sizeof msg);
        int result = recv(vm->debugger.comm_sock, msg, 256, 0);


        if(result == -1) {
            if(errno == EWOULDBLOCK || errno == EAGAIN) return false;
        } else if(result > 0) {
            return true;
        } else if(result == 0) {
            #ifdef _WIN32
                closesocket(vm->debugger.comm_sock);
            #else
                close(vm->debugger.comm_sock);
            #endif
            vm->debugger.comm_sock = -1;
            printf("luxe / wren / debugger / connection closed\n");
        }
    }
    return false;
}

void wrenDebuggerPollConfigCmds(WrenVM* vm) {
    bool got_msg = debuggerPollInput(vm);
    if(got_msg) {
        // printf("Got input config: \"%s\"\n", msg_cursor);
        debuggerProcessConfigCmds(vm, msg_cursor);
    }
}

DebuggerCmd debuggerGetCmd(WrenVM* vm, int ip) {
    bool got_msg = debuggerPollInput(vm);

    if(got_msg) {
        // printf("Got input cmd: \"%s\"\n", msg_cursor);
        bool processed = debuggerProcessConfigCmds(vm, msg_cursor);
        if(!processed) {
            return debuggerProcessStoppedCmds(vm, msg_cursor, ip);
        }
    }
    return WREN_DEBUGGER_CMD_NONE;
}

bool debuggerProcessConfigCmds(WrenVM* vm, const char* msg) {
    int line = -1;
    int status = -1;
    char module[MAX_NAME_LEN];
    status = sscanf(msg, "addp %s %d", module, &line);
    if(status == 2) {
        int added_id = debuggerAddBreakpoint(&vm->debugger, module, line);

        char out[64];
        int out_len = sprintf(out, "created %d\n", added_id);
        debuggerSendEvent(vm->debugger.comm_sock, WREN_DEBUGGER_EVENT_CREATED_BREAKPOINT, out, out_len, true);

        return true;
    }

    status = sscanf(msg, "delp %s %d", module, &line);
    if(status == 2) {
        debuggerRemoveBreakpoint(&vm->debugger, module, line);
        return true;
    }

    if(strncmp(msg, "source", 6) == 0) {
        debuggerSendSource(vm, msg);
        return true;
    }

    

    return false;
}

DebuggerCmd debuggerProcessStoppedCmds(WrenVM* vm, const char* msg, int ip) {
    if(strncmp(msg, "stack", 5) == 0) {
        debuggerSendStack(vm, msg, ip);
    }

    if(strncmp(msg, "var", 3) == 0) {
        debuggerSendVar(vm, msg);
    }

    if(strncmp(msg, "info ", 5) == 0) {
        char info_section[9]; //"function\0" at most
        int stack_idx = -1;
        int matched = sscanf(msg + 5, "%9s %d", info_section, &stack_idx);
        if(matched == 2) {
            if(strncmp(info_section, "module", 6) == 0) {
                debuggerSendModuleInfo(vm, stack_idx);
            } else if(strncmp(info_section, "function", 8) == 0) {
                debuggerSendFunctionInfo(vm, stack_idx); 
            }
        }
    }

        //:todo: this only checks for prefix, not the whole command. Trim and strcmp instead
    if(strncmp(msg, "cont", 4) == 0) return WREN_DEBUGGER_CMD_CONTINUE;
    if(strncmp(msg, "over", 4) == 0) return WREN_DEBUGGER_CMD_STEP_OVER;
    if(strncmp(msg, "into", 4) == 0) return WREN_DEBUGGER_CMD_STEP_INTO;
    if(strncmp(msg, "out", 3) == 0) return WREN_DEBUGGER_CMD_STEP_OUT;
    return WREN_DEBUGGER_CMD_NONE;
}

#endif
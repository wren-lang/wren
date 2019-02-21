#ifndef scheduler_h
#define scheduler_h

#include "wren.h"

// Sets up the API stack to call one of the resume methods on Scheduler.
//
// If [hasArgument] is false, this just sets up the stack to have another
// argument stored in slot 2 and returns. The module must store the argument
// on the stack and then call [schedulerFinishResume] to complete the call.
//
// Otherwise, the call resumes immediately. Releases [fiber] when called.
void schedulerResume(WrenHandle* fiber, bool hasArgument);

void schedulerFinishResume();
void schedulerResumeError(WrenHandle* fiber, const char* error);

void schedulerShutdown();

#endif

#include <stdlib.h>
#include <string.h>

#include "uv.h"

#include "scheduler.h"
#include "wren.h"
#include "vm.h"

// A handle to the "Scheduler" class object. Used to call static methods on it.
static WrenHandle* schedulerClass;

// This method resumes a fiber that is suspended waiting on an asynchronous
// operation. The first resumes it with zero arguments, and the second passes
// one.
static WrenHandle* resume1;
static WrenHandle* resume2;
static WrenHandle* resumeError;

static void resume(WrenHandle* method)
{
  WrenVM* vm = getVM();
  WrenFiber* fiber = wrenGetCurrentFiber(vm);
  WrenInterpretResult result = wrenCall(fiber, method);
  
  // If a runtime error occurs in response to an async operation and nothing
  // catches the error in the fiber, then exit the CLI.
  if (result == WREN_RESULT_RUNTIME_ERROR)
  {
    uv_stop(getLoop());
    setExitCode(70); // EX_SOFTWARE.
  }
}

void schedulerCaptureMethods(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  
  wrenSetSlotCount(fiber, 1);
  wrenGetVariable(vm, "scheduler", "Scheduler", 0);
  schedulerClass = wrenGetSlotHandle(fiber, 0);
  
  resume1 = wrenMakeCallHandle(vm, "resume_(_)");
  resume2 = wrenMakeCallHandle(vm, "resume_(_,_)");
  resumeError = wrenMakeCallHandle(vm, "resumeError_(_,_)");
}

void schedulerResume(WrenHandle* resumedFiber, bool hasArgument)
{
  WrenVM* vm = getVM();
  WrenFiber* fiber = wrenGetCurrentFiber(vm);
  
  wrenSetSlotCount(fiber, 2 + (hasArgument ? 1 : 0));
  wrenSetSlotHandle(fiber, 0, schedulerClass);
  wrenSetSlotHandle(fiber, 1, resumedFiber);
  wrenReleaseHandle(vm, resumedFiber);
  
  // If we don't need to wait for an argument to be stored on the stack, resume
  // it now.
  if (!hasArgument) resume(resume1);
}

void schedulerFinishResume()
{
  resume(resume2);
}

void schedulerResumeError(WrenHandle* resumedFiber, const char* error)
{
  schedulerResume(resumedFiber, true);
  WrenVM* vm = getVM();
  WrenFiber* fiber = wrenGetCurrentFiber(vm);
  wrenSetSlotString(fiber, 2, error);
  resume(resumeError);
}

void schedulerShutdown()
{
  // If the module was never loaded, we don't have anything to release.
  if (schedulerClass == NULL) return;
  
  WrenVM* vm = getVM();
  wrenReleaseHandle(vm, schedulerClass);
  wrenReleaseHandle(vm, resume1);
  wrenReleaseHandle(vm, resume2);
  wrenReleaseHandle(vm, resumeError);
}

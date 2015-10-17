#include <stdlib.h>
#include <string.h>

#include "uv.h"

#include "scheduler.h"
#include "wren.h"
#include "vm.h"

// This method resumes a fiber that is suspended waiting on an asynchronous
// operation. The first resumes it with zero arguments, and the second passes
// one.
static WrenValue* resume;
static WrenValue* resumeWithArg;
static WrenValue* resumeError;

void schedulerCaptureMethods(WrenVM* vm)
{
  resume = wrenGetMethod(vm, "scheduler", "Scheduler", "resume_(_)");
  resumeWithArg = wrenGetMethod(vm, "scheduler", "Scheduler", "resume_(_,_)");
  resumeError = wrenGetMethod(vm, "scheduler", "Scheduler", "resumeError_(_,_)");
}

static void callResume(WrenValue* resumeMethod, WrenValue* fiber,
                       const char* argTypes, ...)
{
  va_list args;
  va_start(args, argTypes);
  WrenInterpretResult result = wrenCallVarArgs(getVM(), resumeMethod, NULL,
                                               argTypes, args);
  va_end(args);
  
  wrenReleaseValue(getVM(), fiber);
  
  // If a runtime error occurs in response to an async operation and nothing
  // catches the error in the fiber, then exit the CLI.
  if (result == WREN_RESULT_RUNTIME_ERROR)
  {
    uv_stop(getLoop());
    setExitCode(70); // EX_SOFTWARE.
  }
}

void schedulerResume(WrenValue* fiber)
{
  callResume(resume, fiber, "v", fiber);
}

void schedulerResumeBytes(WrenValue* fiber, const char* bytes, size_t length)
{
  callResume(resumeWithArg, fiber, "va", fiber, bytes, length);
}

void schedulerResumeDouble(WrenValue* fiber, double value)
{
  callResume(resumeWithArg, fiber, "vd", fiber, value);
}

void schedulerResumeString(WrenValue* fiber, const char* text)
{
  callResume(resumeWithArg, fiber, "vs", fiber, text);
}

void schedulerResumeError(WrenValue* fiber, const char* error)
{
  callResume(resumeError, fiber, "vs", fiber, error);
}

void schedulerShutdown()
{
  if (resume != NULL) wrenReleaseValue(getVM(), resume);
  if (resumeWithArg != NULL) wrenReleaseValue(getVM(), resumeWithArg);
  if (resumeError != NULL) wrenReleaseValue(getVM(), resumeError);
}

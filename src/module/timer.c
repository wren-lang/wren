#include <stdlib.h>
#include <string.h>

#include "uv.h"

#include "timer.h"
#include "wren.h"
#include "vm.h"

// This is generated from builtin/module/timer.wren. Do not edit here.
static const char* timerLibSource =
"class Timer {\n"
"  static sleep(milliseconds) {\n"
"    if (!(milliseconds is Num)) Fiber.abort(\"Milliseconds must be a number.\")\n"
"    if (milliseconds < 0) Fiber.abort(\"Milliseconds cannot be negative.\")\n"
"    startTimer_(milliseconds, Fiber.current)\n"
"\n"
"    runNextScheduled_()\n"
"  }\n"
"\n"
"  // TODO: Once the CLI modules are more fleshed out, find a better place to\n"
"  // put this.\n"
"  static schedule(callable) {\n"
"    if (__scheduled == null) __scheduled = []\n"
"    __scheduled.add(Fiber.new {\n"
"      callable.call()\n"
"      runNextScheduled_()\n"
"    })\n"
"  }\n"
"\n"
"  foreign static startTimer_(milliseconds, fiber)\n"
"\n"
"  // Called by native code.\n"
"  static resumeTimer_(fiber) {\n"
"    fiber.transfer()\n"
"  }\n"
"\n"
"  static runNextScheduled_() {\n"
"    if (__scheduled == null || __scheduled.isEmpty) {\n"
"      Fiber.suspend()\n"
"    } else {\n"
"      __scheduled.removeAt(0).transfer()\n"
"    }\n"
"  }\n"
"}\n"
"\n";

// The Wren method to call when a timer has completed.
static WrenMethod* resumeTimer;

// Called by libuv when the timer finished closing.
static void timerCloseCallback(uv_handle_t* handle)
{
  free(handle);
}

// Called by libuv when the timer has completed.
static void timerCallback(uv_timer_t* handle)
{
  WrenValue* fiber = (WrenValue*)handle->data;
  
  // Tell libuv that we don't need the timer anymore.
  uv_close((uv_handle_t*)handle, timerCloseCallback);

  // Run the fiber that was sleeping.
  wrenCall(getVM(), resumeTimer, "v", fiber);
  wrenReleaseValue(getVM(), fiber);
}

static void startTimer(WrenVM* vm)
{
  // If we haven't looked up the resume method yet, grab it now.
  if (resumeTimer == NULL)
  {
    resumeTimer = wrenGetMethod(vm, "timer", "Timer", "resumeTimer_(_)");
  }
  
  int milliseconds = (int)wrenGetArgumentDouble(vm, 1);
  WrenValue* fiber = wrenGetArgumentValue(vm, 2);
  
  // Store the fiber to resume when the timer completes.
  uv_timer_t* handle = (uv_timer_t*)malloc(sizeof(uv_timer_t));
  handle->data = fiber;
  
  uv_timer_init(getLoop(), handle);
  uv_timer_start(handle, timerCallback, milliseconds, 0);
}

char* timerGetSource()
{
  size_t length = strlen(timerLibSource);
  char* copy = (char*)malloc(length + 1);
  strncpy(copy, timerLibSource, length + 1);
  
  return copy;
}

WrenForeignMethodFn timerBindForeign(
    WrenVM* vm, const char* className, bool isStatic, const char* signature)
{
  if (strcmp(className, "Timer") != 0) return NULL;

  if (isStatic && strcmp(signature, "startTimer_(_,_)") == 0) return startTimer;
  
  return NULL;
}

void timerReleaseMethods()
{
  if (resumeTimer != NULL) wrenReleaseMethod(getVM(), resumeTimer);
}

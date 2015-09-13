#include <stdlib.h>
#include <string.h>

#include "uv.h"

#include "timer.h"
#include "wren.h"
#include "vm.h"

#include "timer.wren.inc"

// The Wren method to call when a timer has completed.
static WrenValue* resumeTimer;

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
  wrenCall(getVM(), resumeTimer, NULL, "v", fiber);
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
  size_t length = strlen(timerModuleSource);
  char* copy = (char*)malloc(length + 1);
  strncpy(copy, timerModuleSource, length + 1);

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
  if (resumeTimer != NULL) wrenReleaseValue(getVM(), resumeTimer);
}

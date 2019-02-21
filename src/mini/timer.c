#include <stdlib.h>
#include <string.h>

#include "scheduler.h"
#include "vm.h"
#include "wren.h"
#include "userev.h"
#include <stdio.h>


// Called by libuv when the timer finished closing.
static void timerCloseCallback(void* handle)
{
  free(handle);
}

// Called by user event library when the timer has completed.
static void timerCallback(struct userevUserData* handle)
{
  WrenHandle* fiber = (WrenHandle*)handle->data;

  //
  // stop other timer callbacks
  userevStopTimer(timerCloseCallback);
  
  // Run the fiber that was sleeping.
  schedulerResume(fiber, false);
}

void timerStartTimer(WrenVM* vm)
{
  int milliseconds = (int)wrenGetSlotDouble(vm, 1);
  WrenHandle* fiber = wrenGetSlotHandle(vm, 2);

// Store the fiber to resume when the timer completes.
  struct userevUserData* handle = (struct userevUserData *)malloc(sizeof(struct userevUserData));
  handle->data = fiber;

  userevStartTimer(handle, timerCallback, milliseconds, 0);

}

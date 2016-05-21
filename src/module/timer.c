#include <stdlib.h>
#include <string.h>

#include "uv.h"

#include "scheduler.h"
#include "vm.h"
#include "wren.h"

// Called by libuv when the timer finished closing.
static void timerCloseCallback(uv_handle_t* handle)
{
  free(handle);
}

// Called by libuv when the timer has completed.
static void timerCallback(uv_timer_t* handle)
{
  WrenHandle* fiber = (WrenHandle*)handle->data;

  // Tell libuv that we don't need the timer anymore.
  uv_close((uv_handle_t*)handle, timerCloseCallback);

  // Run the fiber that was sleeping.
  schedulerResume(fiber, false);
}

void timerStartTimer(WrenVM* vm)
{
  int milliseconds = (int)wrenGetSlotDouble(vm, 1);
  WrenHandle* fiber = wrenGetSlotHandle(vm, 2);

  // Store the fiber to resume when the timer completes.
  uv_timer_t* handle = (uv_timer_t*)malloc(sizeof(uv_timer_t));
  handle->data = fiber;

  uv_timer_init(getLoop(), handle);
  uv_timer_start(handle, timerCallback, milliseconds, 0);
}

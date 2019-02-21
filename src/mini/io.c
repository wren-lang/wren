#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "scheduler.h"
#include "vm.h"
#include "wren.h"

void stdoutFlush(WrenVM* vm)
{
  fflush(stdout);
  wrenSetSlotNull(vm, 0);
}

void ioShutdown(){}

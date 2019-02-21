#include "userev.h"
#include <stdio.h>

// abstraction of user event library like libuv

void userevLoopInit()
{
  printf("TODO implement user event loop init\n");
}

void userevLoop()
{
  printf("TODO implement user event loop\n");
}

void userevLoopClose()
{
  printf("TODO implement user event close\n");  
}

void userevReset()
{
  printf("TODO implement user event reset\n");  
}

void userevLoopStop()
{
  printf("TODO implement user event stop\n");  
}

void userevStartTimer(struct userevUserData* handle,
		      void (*timerCallback)(struct userevUserData* handle),
		      int interval,
		      int misc)
{
  printf("TODO implement user event start timer\n");
}
void userevStopTimer(
		     void (*timerCloseCallback)(void* handle)
)
{
  printf("TODO implement user event stop timer\n");
}

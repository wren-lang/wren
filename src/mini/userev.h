#pragma once

// abstraction of user event library like libuv

void userevLoopInit();
void userevLoop();
void userevLoopClose();
void userevLoopStop();
void userevReset();

struct userevUserData {
  void* data;
};
void userevStartTimer(struct userevUserData* handle,
		      void (*timerCallback)(struct userevUserData* handle),
		      int interval,
		      int misc);
void userevStopTimer(
		     void (*timerCloseCallback)(void* handle)
);



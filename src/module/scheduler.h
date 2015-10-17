#ifndef scheduler_h
#define scheduler_h

#include "wren.h"

void schedulerResume(WrenValue* fiber);
void schedulerResumeBytes(WrenValue* fiber, const char* bytes, size_t length);
void schedulerResumeDouble(WrenValue* fiber, double value);
void schedulerResumeString(WrenValue* fiber, const char* text);
void schedulerResumeError(WrenValue* fiber, const char* error);

void schedulerShutdown();

#endif

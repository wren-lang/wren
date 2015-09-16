#ifndef scheduler_h
#define scheduler_h

#include "wren.h"

WrenForeignMethodFn schedulerBindForeign(
    WrenVM* vm, const char* className, bool isStatic, const char* signature);

void schedulerResume(WrenValue* fiber);
void schedulerResumeDouble(WrenValue* fiber, double value);
void schedulerResumeString(WrenValue* fiber, const char* text);

void schedulerReleaseMethods();

#endif

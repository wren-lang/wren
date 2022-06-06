
# The debugger is work in progress

This branch is very experimental and not finished.
There's a lot of exploratory code that isn't well defined or cleaned up.
This is expected.

We welcome contribution to make it more complete.
Please open a discussion to discuss major changes before doing them.

https://github.com/wren-lang/wren/issues/425

# Testing the debugger
- enable `#define WREN_DEBUGGER 1` in include/wren.h
- enable `#define WREN_DEBUGGER 1` in wren_common.h
- call `wrenDebuggerPollConfigCmds(vm)` in e.g a main loop
- implement the config stuff needed

```c

const char* get_module_path(WrenVM* vm, const char* module, const char* root) 
{
    //return a path on disk for vscode to open when stepping into code etc
}

/// ....

//config
config.modulePathFn = get_module_path;
config.debuggerPort = "8089";
config.enableDebugger = true;
```
  
# Accessing the debugger
The debugger operates a simple protocol over a network socket. 
You can connect to it with telnet for example and control it.

The vscode extension below integrates with it for a bigger example.

# The (very wip) vscode extension

Debugging shown in this video in vscode was made possible using this extension: 
https://github.com/wren-lang/wren-vscode

The extension is also very rough and experimental.

# old original todo notes from @KeyMaster-

- [ ] Patch method field indices in ObjModule->fieldSlots
- [x] Make it run on windows (specifically socket.h and related)
- [ ] Conform to wren code style
- [ ] Better naming of things and protocol messages
- [ ] Local variables which are only passed to other functions aren't being tracked
- [ ] Test behaviour on fiber switches 
- [ ] Expand upvalue valid range to the whole function they are defined in 
- [ ] Static variables not used in a function are not visible to the debugger 

**Smaller details**
- [ ] Remove breakpoints maximum? would add dynamic allocation though. 
- [ ] Remove max length of module names in breakpoint struct
- [ ] Stack excludes foreign calls and core module calls, decide if this should be done. (affects getStackIdxFrame)
- [ ] Stack send, uses `|` to delineate but module string could contain `|`. Escape or pass module name string length.
- [ ] Check memory management of modulePathFn, requires free on caller side 
- [ ] loadModuleFn gets called again by the debugger, update corresponding comment. (Or cache resuls?) 
- [ ] Max length of scanned var name in debuggerSendVar does not use MAX_VARIABLE_NAME macro (to insert it into the string we require more macros though) 
- [ ] Should NaN be treated as Num type? 
- [ ] Maybe get rid of static char buffer for printing for send var functions? 
- [ ] debugger commands accept any text with the command as a prefix 
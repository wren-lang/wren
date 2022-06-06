
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


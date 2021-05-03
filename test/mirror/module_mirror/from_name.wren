
import "mirror" for ModuleMirror

// Check `core` module handling
var moduleMirror = ModuleMirror.fromName("core")

System.print(moduleMirror is ModuleMirror ) // expect: true
System.print(moduleMirror.name ) // expect: core

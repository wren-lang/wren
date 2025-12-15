
import "mirror" for Mirror, ModuleMirror

var mirror = Mirror.reflect(Class)

System.print(mirror.moduleMirror == ModuleMirror.fromName("core")) // expect: true

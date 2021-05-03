
import "mirror" for Mirror, ModuleMirror

class Foo {
}

var mirror = Mirror.reflect(Foo)

System.print(mirror.moduleMirror == ModuleMirror.current) // expect: true

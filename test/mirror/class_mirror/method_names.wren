
import "mirror" for Mirror

class Foo {
  construct new() { }
}

var ObjectMirror = Mirror.reflect(Object)
var FooMirror = Mirror.reflect(Foo)

System.print(FooMirror.methodNames.count ==
             ObjectMirror.methodNames.count + 1)   // expect: true


import "mirror" for Mirror

class Foo {
  construct new() {}
  call() {
    return System.print("()")
  }
  call(arg0) {
    return System.print("(%(arg0))")
  }
  call(arg0, arg1) {
    return System.print("(%(arg0), %(arg1))")
  }
  call(arg0, arg1, arg2) {
    return System.print("(%(arg0), %(arg1), %(arg2))")
  }
  call(arg0, arg1, arg2, arg3) {
    return System.print("(%(arg0), %(arg1), %(arg2), %(arg3))")
  }
}

var foo = Foo.new()

var FooMirror = Mirror.reflect(Foo)

FooMirror.methodMirrors["call()"].call(foo) // expect: ()
FooMirror.methodMirrors["call(_)"].call(foo, "arg0") // expect: (arg0)
FooMirror.methodMirrors["call(_,_)"].call(foo, "arg0", "arg1") // expect: (arg0, arg1)
FooMirror.methodMirrors["call(_,_,_)"].call(foo, "arg0", "arg1", "arg2") // expect: (arg0, arg1, arg2)
FooMirror.methodMirrors["call(_,_,_,_)"].call(foo, "arg0", "arg1", "arg2", "arg3") // expect: (arg0, arg1, arg2, arg3)

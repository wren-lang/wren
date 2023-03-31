class Foo {
  construct new()        { System.print("on constructor") }

  static getter          { "on static getter" }
  static setter=(value)  { "on static setter=%(value)" }

  static [index]         { "on static [index]" }
  static [index]=(value) { "on static [index]=%(value)" }

  static method()        { "on static method()" }
  static method(arg)     { "on static method(%(arg))" }
}

Foo.@"new()"()                                 // expect: on constructor

System.print(Foo.@"getter"())                  // expect: on static getter
System.print(Foo.@"setter=(_)"("value"))       // expect: on static setter=value

System.print(Foo.@"[_]"("index"))              // expect: on static [index]
System.print(Foo.@"[_]=(_)"("index", "value")) // expect: on static [index]=value

System.print(Foo.@"method()"())                // expect: on static method()
System.print(Foo.@"method(_)"("arg"))          // expect: on static method(arg)

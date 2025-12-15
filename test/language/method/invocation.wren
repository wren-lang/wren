class Foo {
  construct new() {}

  getter          { "on getter" }
  setter=(value)  { "on setter=%(value)" }

  [index]         { "on [index]" }
  [index]=(value) { "on [index]=%(value)" }

  method()        { "on method()" }
  method(arg)     { "on method(%(arg))" }
}

var foo = Foo.new()

System.print(foo.@"getter"())                  // expect: on getter
System.print(foo.@"setter=(_)"("value"))       // expect: on setter=value

System.print(foo.@"[_]"("index"))              // expect: on [index]
System.print(foo.@"[_]=(_)"("index", "value")) // expect: on [index]=value

System.print(foo.@"method()"())                // expect: on method()
System.print(foo.@"method(_)"("arg"))          // expect: on method(arg)

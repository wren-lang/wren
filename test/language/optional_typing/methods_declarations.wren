
class Foo {
  static method(): Foo { Foo.new() }
  static method(arg: Num): Foo { Foo.new() }
  
  construct new() { }
  
  method(): Foo { Foo.new() }
  method(arg: Num): Foo { Foo.new() }
  
  [i : Num]: Foo { Foo.new() }
  [i : Num] = (val: Num): Foo { Foo.new() }
}

System.print(Foo.method()) // expect: instance of Foo
System.print(Foo.method(42)) // expect: instance of Foo

var bar = Foo.new()
System.print(bar.method()) // expect: instance of Foo
System.print(bar.method(42)) // expect: instance of Foo

System.print(bar[42]) // expect: instance of Foo
System.print(bar[42] = 42) // expect: instance of Foo

var variable = "before"
System.print(variable) // expect: before
variable = "after"
System.print(variable) // expect: after

class Foo {
  static method {
    variable = "method"
  }
}

Foo.method
System.print(variable) // expect: method

Fn.new {
  variable = "fn"
}.call()
System.print(variable) // expect: fn

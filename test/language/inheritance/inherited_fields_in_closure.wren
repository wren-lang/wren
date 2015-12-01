class Foo {
  construct new() { _field = "Foo field" }

  closeOverFooGet() {
    return Fn.new { Fn.new { _field } }
  }

  closeOverFooSet() {
    return Fn.new { Fn.new { _field = "new foo value" } }
  }
}

class Bar is Foo {
  construct new() {
    super()
    _field = "Bar field"
  }

  closeOverBarGet() {
    return Fn.new { Fn.new { _field } }
  }

  closeOverBarSet() {
    return Fn.new { Fn.new { _field = "new bar value" } }
  }
}

var bar = Bar.new()
System.print(bar.closeOverFooGet()()()) // expect: Foo field
System.print(bar.closeOverBarGet()()()) // expect: Bar field
bar.closeOverFooSet()()()
System.print(bar.closeOverFooGet()()()) // expect: new foo value
System.print(bar.closeOverBarGet()()()) // expect: Bar field
bar.closeOverBarSet()()()
System.print(bar.closeOverFooGet()()()) // expect: new foo value
System.print(bar.closeOverBarGet()()()) // expect: new bar value

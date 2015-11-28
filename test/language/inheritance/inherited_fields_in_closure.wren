class Foo {
  construct new() { _field = "Foo field" }

  closeOverFooGet() {
    return fn () { fn () { _field } }
  }

  closeOverFooSet() {
    return fn () { fn () { _field = "new foo value" } }
  }
}

class Bar is Foo {
  construct new() {
    super()
    _field = "Bar field"
  }

  closeOverBarGet() {
    return fn () { fn () { _field } }
  }

  closeOverBarSet() {
    return fn () { fn () { _field = "new bar value" } }
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

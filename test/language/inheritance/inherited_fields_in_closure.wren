class Foo {
  def construct new() { _field = "Foo field" }

  def closeOverFooGet {
    return Fn.new { Fn.new { _field } }
  }

  def closeOverFooSet {
    return Fn.new { Fn.new { _field = "new foo value" } }
  }
}

class Bar is Foo {
  def construct new() {
    super()
    _field = "Bar field"
  }

  def closeOverBarGet {
    return Fn.new { Fn.new { _field } }
  }

  def closeOverBarSet {
    return Fn.new { Fn.new { _field = "new bar value" } }
  }
}

var bar = Bar.new()
System.print(bar.closeOverFooGet.call().call()) // expect: Foo field
System.print(bar.closeOverBarGet.call().call()) // expect: Bar field
bar.closeOverFooSet.call().call()
System.print(bar.closeOverFooGet.call().call()) // expect: new foo value
System.print(bar.closeOverBarGet.call().call()) // expect: Bar field
bar.closeOverBarSet.call().call()
System.print(bar.closeOverFooGet.call().call()) // expect: new foo value
System.print(bar.closeOverBarGet.call().call()) // expect: new bar value

class Foo {
  new { _field = "Foo field" }

  closeOverFooGet {
    return fn { return fn { return _field } }
  }

  closeOverFooSet {
    return fn { return fn { _field = "new foo value" } }
  }
}

class Bar is Foo {
  new {
    super
    _field = "Bar field"
  }

  closeOverBarGet {
    return fn { return fn { return _field } }
  }

  closeOverBarSet {
    return fn { return fn { _field = "new bar value" } }
  }
}

var bar = new Bar
IO.print(bar.closeOverFooGet.call.call) // expect: Foo field
IO.print(bar.closeOverBarGet.call.call) // expect: Bar field
bar.closeOverFooSet.call.call
IO.print(bar.closeOverFooGet.call.call) // expect: new foo value
IO.print(bar.closeOverBarGet.call.call) // expect: Bar field
bar.closeOverBarSet.call.call
IO.print(bar.closeOverFooGet.call.call) // expect: new foo value
IO.print(bar.closeOverBarGet.call.call) // expect: new bar value

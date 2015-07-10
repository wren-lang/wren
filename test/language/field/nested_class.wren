class Outer {
  method {
    _field = "outer"
    IO.print(_field) // expect: outer

    class Inner {
      method {
        _field = "inner"
        IO.print(_field) // expect: inner
      }
    }

    Inner.new().method
    IO.print(_field) // expect: outer
  }
}

Outer.new().method

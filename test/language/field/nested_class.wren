class Outer {
  construct new() {}

  method {
    _field = "outer"
    IO.print(_field) // expect: outer

    class Inner {
      construct new() {}

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

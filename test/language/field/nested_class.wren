class Outer {
  construct new() {}

  method {
    _field = "outer"
    System.print(_field) // expect: outer

    class Inner {
      construct new() {}

      method {
        _field = "inner"
        System.print(_field) // expect: inner
      }
    }

    Inner.new().method
    System.print(_field) // expect: outer
  }
}

Outer.new().method

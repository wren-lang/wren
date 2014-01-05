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

    (new Inner).method
    IO.print(_field) // expect: outer
  }
}

(new Outer).method

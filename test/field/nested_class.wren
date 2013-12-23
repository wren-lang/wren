class Outer {
  method {
    _field = "outer"
    IO.write(_field) // expect: outer

    class Inner {
      method {
        _field = "inner"
        IO.write(_field) // expect: inner
      }
    }

    (new Inner).method
    IO.write(_field) // expect: outer
  }
}

(new Outer).method

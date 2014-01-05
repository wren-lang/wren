class Foo {
  method {
    return "ok"
    IO.print("bad")
  }
}

IO.print((new Foo).method) // expect: ok

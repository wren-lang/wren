class Foo {
  method {
    return "ok"
    IO.write("bad")
  }
}

IO.write((new Foo).method) // expect: ok

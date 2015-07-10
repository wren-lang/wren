class Foo {
  method {
    return "ok"
    IO.print("bad")
  }
}

IO.print(Foo.new().method) // expect: ok

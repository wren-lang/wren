class Foo {
  construct new() {}

  method {
    return "ok"
    IO.print("bad")
  }
}

IO.print(Foo.new().method) // expect: ok

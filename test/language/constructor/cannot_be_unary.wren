class Foo {
  this ! { // expect error
    IO.print("ok")
  }
}

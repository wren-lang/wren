class Foo {
  construct new() {}

  bar { "getter" }

  test() {
    var bar = "local"
    System.print(@bar) // expect: getter
  }
}

Foo.new().test()

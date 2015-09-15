class Foo {
  construct new() {}

  bar { "getter" }

  test {
    System.print(bar) // expect: getter

    {
      System.print(bar) // expect: getter
      var bar = "local"
      System.print(bar) // expect: local
    }

    System.print(bar) // expect: getter
  }
}

Foo.new().test

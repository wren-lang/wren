class Foo {
  bar { "getter" }

  test {
    IO.print(bar) // expect: getter

    {
      IO.print(bar) // expect: getter
      var bar = "local"
      IO.print(bar) // expect: local
    }

    IO.print(bar) // expect: getter
  }
}

(new Foo).test

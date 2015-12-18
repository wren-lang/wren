class Foo {
  construct new() {}

  def bar { "getter" }

  def test {
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

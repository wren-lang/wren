class Outer {
  construct new() {
    // TODO: Remove "this." when capitalized names are no longer top-level.
    System.print(this.Inner)           // expect: Inner
    System.print(this.Inner.new().foo) // expect: Inner foo

    System.print(foo) // expect: Outer foo
  }

  class Inner {
    construct new() {}
    def foo { "Inner foo" }
  }

  def foo { "Outer foo" }
}

Outer.new()

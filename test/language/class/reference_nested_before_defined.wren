class Outer {
  // TODO: Remove "this." when capitalized names are no longer top-level.
  this.Inner.new() // expect runtime error: Null does not implement 'new()'.

  class Inner {
    construct new() {}
  }

  construct new() {}
}

Outer.new()

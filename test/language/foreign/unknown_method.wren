class Foo {
  def foreign someUnknownMethod // expect runtime error: Could not find foreign method 'someUnknownMethod' for class Foo in module 'main'.
}

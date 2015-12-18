class Foo {
  foreign def someUnknownMethod // expect runtime error: Could not find foreign method 'someUnknownMethod' for class Foo in module 'main'.
}

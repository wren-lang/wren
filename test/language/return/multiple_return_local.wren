class Foo {
  construct new() {}

  method {
    return "one", "two", "three"
    System.print("bad")
  }

  local_values() {
    var v1, v2, v3 = method
    System.print(v1) // expect: one
    System.print(v2) // expect: two
    System.print(v3) // expect: three
  }
}

Foo.new().local_values()

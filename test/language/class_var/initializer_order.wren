class Foo {
  System.print("one")

  construct new() {
    System.print("six")
  }

  var bar = System.print("two")

  System.print("three")

  var baz = System.print("four")

  System.print("five")
}

var foo = Foo.new()
// expect: one
// expect: two
// expect: three
// expect: four
// expect: five
// expect: six

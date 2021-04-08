
class Foo {
  static call(
        ) {
    System.print("Success") // expect: Success
  }

  construct new () {}

  call(
        ) {
    System.print("Success") // expect: Success
  }
}

Foo.call(
)
Foo.new().call(
)

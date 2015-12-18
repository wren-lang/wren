class Foo {
  def construct new() {}
}
System.print(Foo.new().toString == "instance of Foo") // expect: true

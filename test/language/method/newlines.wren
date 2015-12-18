class Foo {
  def construct new() {}
  def method(a, b) { "method %(a) %(b)" }
  def [a, b] { "subscript %(a) %(b)" }
}

var foo = Foo.new()

// Allow newlines after commas and before ")".
System.print(foo.method("a",

    "b"

    )) // expect: method a b

// Allow newlines after commas and before "]".
System.print(foo["a",

    "b"

    ]) // expect: subscript a b

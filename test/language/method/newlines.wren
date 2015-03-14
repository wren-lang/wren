class Foo {
  method(a, b) { "method " + a + " " + b }
  [a, b] { "subscript " + a + " " + b }
}

var foo = new Foo

// Allow newlines after commas and before ")".
IO.print(foo.method("a",

    "b"

    )) // expect: method a b

// Allow newlines after commas and before "]".
IO.print(foo["a",

    "b"

    ]) // expect: subscript a b

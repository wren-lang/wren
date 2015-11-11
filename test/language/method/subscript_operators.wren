class Foo {
  construct new() {}
  [a] { "1-subscript %(a)" }
  [a, b] { "2-subscript %(a) %(b)" }
  [a, b, c] { "3-subscript %(a) %(b) %(c)" }
  [a]=(value) { "1-subscript setter %(a) = %(value)" }
  [a, b]=(value) { "2-subscript setter %(a) %(b) = %(value)" }
  [a, b, c]=(value) { "3-subscript setter %(a) %(b) %(c) = %(value)" }
}

var foo = Foo.new()
System.print(foo["a"]) // expect: 1-subscript a
System.print(foo["a", "b"]) // expect: 2-subscript a b
System.print(foo["a", "b", "c"]) // expect: 3-subscript a b c
System.print(foo["a"] = "value") // expect: 1-subscript setter a = value
System.print(foo["a", "b"] = "value") // expect: 2-subscript setter a b = value
System.print(foo["a", "b", "c"] = "value") // expect: 3-subscript setter a b c = value

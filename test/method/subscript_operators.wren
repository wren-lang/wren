class Foo {
  [a] { "1-subscript " + a }
  [a, b] { "2-subscript " + a + " " + b }
  [a, b, c] { "3-subscript " + a + " " + b + " " + c }
  [a]=(value) { "1-subscript setter " + a + " = " + value }
  [a, b]=(value) { "2-subscript setter " + a + " " + b + " = " + value }
  [a, b, c]=(value) { "3-subscript setter " + a + " " + b + " " + c + " = " + value }
}

var foo = new Foo
IO.print(foo["a"]) // expect: 1-subscript a
IO.print(foo["a", "b"]) // expect: 2-subscript a b
IO.print(foo["a", "b", "c"]) // expect: 3-subscript a b c
IO.print(foo["a"] = "value") // expect: 1-subscript setter a = value
IO.print(foo["a", "b"] = "value") // expect: 2-subscript setter a b = value
IO.print(foo["a", "b", "c"] = "value") // expect: 3-subscript setter a b c = value

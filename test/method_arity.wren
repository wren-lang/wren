class Foo {
  method { 0 }
  method(a) { a }
  method(a, b) { a + b }
  method(a, b, c) { a + b + c }
  method(a, b, c, d) { a + b + c + d }
  method(a, b, c, d, e) { a + b + c + d + e }
  method(a, b, c, d, e, f) { a + b + c + d + e + f }
  method(a, b, c, d, e, f, g) { a + b + c + d + e + f + g }
  method(a, b, c, d, e, f, g, h) { a + b + c + d + e + f + g + h }
  method(a, b, c, d, e, f, g, h, i) { a + b + c + d + e + f + g + h + i }
  method(a, b, c, d, e, f, g, h, i, j) { a + b + c + d + e + f + g + h + i + j }
}

var foo = Foo.new
io.write(foo.method) // expect: 0
io.write(foo.method(1)) // expect: 1
io.write(foo.method(1, 2)) // expect: 3
io.write(foo.method(1, 2, 3)) // expect: 6
io.write(foo.method(1, 2, 3, 4)) // expect: 10
io.write(foo.method(1, 2, 3, 4, 5)) // expect: 15
io.write(foo.method(1, 2, 3, 4, 5, 6)) // expect: 21
io.write(foo.method(1, 2, 3, 4, 5, 6, 7)) // expect: 28
io.write(foo.method(1, 2, 3, 4, 5, 6, 7, 8)) // expect: 36
io.write(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9)) // expect: 45
io.write(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)) // expect: 55

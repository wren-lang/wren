class Foo {
  construct new() {}
  def method { "getter" }
  def method() { "no args" }
  def method(a) { a }
  def method(a, b) { a + b }
  def method(a, b, c) { a + b + c }
  def method(a, b, c, d) { a + b + c + d }
  def method(a, b, c, d, e) { a + b + c + d + e }
  def method(a, b, c, d, e, f) { a + b + c + d + e + f }
  def method(a, b, c, d, e, f, g) { a + b + c + d + e + f + g }
  def method(a, b, c, d, e, f, g, h) { a + b + c + d + e + f + g + h }
  def method(a, b, c, d, e, f, g, h, i) { a + b + c + d + e + f + g + h + i }
  def method(a, b, c, d, e, f, g, h, i, j) { a + b + c + d + e + f + g + h + i + j }
  def method(a, b, c, d, e, f, g, h, i, j, k) { a + b + c + d + e + f + g + h + i + j + k}
  def method(a, b, c, d, e, f, g, h, i, j, k, l) { a + b + c + d + e + f + g + h + i + j + k + l}
  def method(a, b, c, d, e, f, g, h, i, j, k, l, m) { a + b + c + d + e + f + g + h + i + j + k + l + m}
  def method(a, b, c, d, e, f, g, h, i, j, k, l, m, n) { a + b + c + d + e + f + g + h + i + j + k + l + m + n}
  def method(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) { a + b + c + d + e + f + g + h + i + j + k + l + m + n + o}
  def method(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) { a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p}
}

var foo = Foo.new()
System.print(foo.method) // expect: getter
System.print(foo.method()) // expect: no args
System.print(foo.method(1)) // expect: 1
System.print(foo.method(1, 2)) // expect: 3
System.print(foo.method(1, 2, 3)) // expect: 6
System.print(foo.method(1, 2, 3, 4)) // expect: 10
System.print(foo.method(1, 2, 3, 4, 5)) // expect: 15
System.print(foo.method(1, 2, 3, 4, 5, 6)) // expect: 21
System.print(foo.method(1, 2, 3, 4, 5, 6, 7)) // expect: 28
System.print(foo.method(1, 2, 3, 4, 5, 6, 7, 8)) // expect: 36
System.print(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9)) // expect: 45
System.print(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)) // expect: 55
System.print(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)) // expect: 66
System.print(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12)) // expect: 78
System.print(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13)) // expect: 91
System.print(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14)) // expect: 105
System.print(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)) // expect: 120
System.print(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)) // expect: 136

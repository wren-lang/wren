class Foo {
  method { return 0 }
  method(a) { return a }
  method(a, b) { return a + b }
  method(a, b, c) { return a + b + c }
  method(a, b, c, d) { return a + b + c + d }
  method(a, b, c, d, e) { return a + b + c + d + e }
  method(a, b, c, d, e, f) { return a + b + c + d + e + f }
  method(a, b, c, d, e, f, g) { return a + b + c + d + e + f + g }
  method(a, b, c, d, e, f, g, h) { return a + b + c + d + e + f + g + h }
  method(a, b, c, d, e, f, g, h, i) { return a + b + c + d + e + f + g + h + i }
  method(a, b, c, d, e, f, g, h, i, j) { return a + b + c + d + e + f + g + h + i + j }
  method(a, b, c, d, e, f, g, h, i, j, k) { return a + b + c + d + e + f + g + h + i + j + k}
  method(a, b, c, d, e, f, g, h, i, j, k, l) { return a + b + c + d + e + f + g + h + i + j + k + l}
  method(a, b, c, d, e, f, g, h, i, j, k, l, m) { return a + b + c + d + e + f + g + h + i + j + k + l + m}
  method(a, b, c, d, e, f, g, h, i, j, k, l, m, n) { return a + b + c + d + e + f + g + h + i + j + k + l + m + n}
  method(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) { return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o}
  method(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) { return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p}
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
io.write(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)) // expect: 66
io.write(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12)) // expect: 78
io.write(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13)) // expect: 91
io.write(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14)) // expect: 105
io.write(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)) // expect: 120
io.write(foo.method(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)) // expect: 136

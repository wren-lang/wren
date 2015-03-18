// skip: Range subscripts for strings don't handle UTF-8.
var a = "123"
a[0...-5] // expect runtime error: Range end out of bounds.

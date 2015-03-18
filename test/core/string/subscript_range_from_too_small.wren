// skip: Range subscripts for strings don't handle UTF-8.
var a = "123"
a[-4..2] // expect runtime error: Range start out of bounds.

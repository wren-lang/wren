var str = "string"

IO.print(str.join("") == str) // expect: true

IO.print(str.join(", ")) // expect: s, t, r, i, n, g

// 8-bit clean
var ing = "a\0b\0c"
IO.print(ing.join("") == ing) // expect: true
IO.print(ing.join(", ")) // expect: a,

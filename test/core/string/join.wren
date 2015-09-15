var str = "string"

System.print(str.join("") == str) // expect: true

System.print(str.join(", ")) // expect: s, t, r, i, n, g

// 8-bit clean.
var ing = "a\0b\0c"
System.print(ing.join("") == ing) // expect: true
System.print(ing.join(", ") == "a, \0, b, \0, c") // expect: true

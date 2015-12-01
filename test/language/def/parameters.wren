def f0() { 0 }
System.print(f0()) // expect: 0

def f1(a) { a }
System.print(f1(1)) // expect: 1

def f2(a, b) { a + b }
System.print(f2(1, 2)) // expect: 3

def f3(a, b, c) { a + b + c }
System.print(f3(1, 2, 3)) // expect: 6

def f4(a, b, c, d) { a + b + c + d }
System.print(f4(1, 2, 3, 4)) // expect: 10

def f5(a, b, c, d, e) { a + b + c + d + e }
System.print(f5(1, 2, 3, 4, 5)) // expect: 15

def f6(a, b, c, d, e, f) { a + b + c + d + e + f }
System.print(f6(1, 2, 3, 4, 5, 6)) // expect: 21

def f7(a, b, c, d, e, f, g) { a + b + c + d + e + f + g }
System.print(f7(1, 2, 3, 4, 5, 6, 7)) // expect: 28

def f8(a, b, c, d, e, f, g, h) { a + b + c + d + e + f + g + h }
System.print(f8(1, 2, 3, 4, 5, 6, 7, 8)) // expect: 36

def f9(a, b, c, d, e, f, g, h, i) { a + b + c + d + e + f + g + h + i }
System.print(f9(1, 2, 3, 4, 5, 6, 7, 8, 9)) // expect: 45

def f10(a, b, c, d, e, f, g, h, i, j) { a + b + c + d + e + f + g + h + i + j }
System.print(f10(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)) // expect: 55

def f11(a, b, c, d, e, f, g, h, i, j, k) { a + b + c + d + e + f + g + h + i + j + k }
System.print(f11(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)) // expect: 66

def f12(a, b, c, d, e, f, g, h, i, j, k, l) { a + b + c + d + e + f + g + h + i + j + k + l }
System.print(f12(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12)) // expect: 78

def f13(a, b, c, d, e, f, g, h, i, j, k, l, m) { a + b + c + d + e + f + g + h + i + j + k + l + m }
System.print(f13(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13)) // expect: 91

def f14(a, b, c, d, e, f, g, h, i, j, k, l, m, n) { a + b + c + d + e + f + g + h + i + j + k + l + m + n }
System.print(f14(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14)) // expect: 105

def f15(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) { a + b + c + d + e + f + g + h + i + j + k + l + m + n + o }
System.print(f15(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)) // expect: 120

def f16(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) { a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p }
System.print(f16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)) // expect: 136

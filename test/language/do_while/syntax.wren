// Single-expression body.
var c = 0
do System.print(c = c + 1) while (c < 3)
// expect: 1
// expect: 2
// expect: 3

// Block body.
var a = 0
do {
  System.print(a)
  a = a + 1
} while (a < 3)
// expect: 0
// expect: 1
// expect: 2

// no "while" = implicit do..while(false)
do {
  System.print("once")
}
// expect: once
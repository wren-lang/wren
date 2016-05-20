import "io" for Stdin

// Defaults to false.
System.print(Stdin.isRaw) // expect: false

// stdin: abcdefgh
for (i in 1..4) {
  System.print(Stdin.readByte())
}
// expect: 97
// expect: 98
// expect: 99
// expect: 100

Stdin.isRaw = true
System.print(Stdin.isRaw) // expect: true

for (i in 1..4) {
  System.print(Stdin.readByte())
}
// expect: 101
// expect: 102
// expect: 103
// expect: 104

// TODO: This doesn't actually detect a visible difference between raw and
// non-raw mode. Maybe add support to the test runner for writing non-printing
// characters to stdin?

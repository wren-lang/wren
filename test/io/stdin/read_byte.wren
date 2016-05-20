import "io" for Stdin

for (i in 1...13) {
  System.print(Stdin.readByte())
}

// stdin: first
// expect: 102
// expect: 105
// expect: 114
// expect: 115
// expect: 116
// expect: 10

// stdin: second
// expect: 115
// expect: 101
// expect: 99
// expect: 111
// expect: 110
// expect: 100

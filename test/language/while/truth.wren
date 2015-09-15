// False and null are false.
while (false) {
  System.print("bad")
  break
}

while (null) {
  System.print("bad")
  break
}

// Everything else is true.
while (true) {
  System.print("true") // expect: true
  break
}

while (0) {
  System.print(0) // expect: 0
  break
}

while ("") {
  System.print("string") // expect: string
  break
}

// False and null are false.
while (false) {
  IO.print("bad")
  break
}

while (null) {
  IO.print("bad")
  break
}

// Everything else is true.
while (true) {
  IO.print("true") // expect: true
  break
}

while (0) {
  IO.print(0) // expect: 0
  break
}

while ("") {
  IO.print("string") // expect: string
  break
}

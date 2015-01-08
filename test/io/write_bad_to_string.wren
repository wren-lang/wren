class BadToString {
  toString { 3 }
}

IO.write(new BadToString) // expect: [invalid toString]
IO.print
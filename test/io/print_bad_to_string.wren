class BadToString {
  toString { 3 }
}

IO.print(new BadToString) // expect: [invalid toString]

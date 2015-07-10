class BadToString {
  toString { 3 }
}

IO.print(BadToString.new()) // expect: [invalid toString]

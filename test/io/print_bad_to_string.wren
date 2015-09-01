class BadToString {
  construct new() {}
  toString { 3 }
}

IO.print(BadToString.new()) // expect: [invalid toString]

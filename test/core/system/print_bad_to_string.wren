class BadToString {
  construct new() {}
  toString { 3 }
}

System.print(BadToString.new()) // expect: [invalid toString]

class BadToString {
  construct new() {}
  toString { 3 }
}

System.write(BadToString.new())
System.print("!") // expect: [invalid toString]!

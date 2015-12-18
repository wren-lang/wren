class BadToString {
  construct new() {}
  def toString { 3 }
}

System.write(BadToString.new())
System.print("!") // expect: [invalid toString]!

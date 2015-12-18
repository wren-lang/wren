class BadToString {
  def construct new() {}
  def toString { 3 }
}

System.print(BadToString.new()) // expect: [invalid toString]

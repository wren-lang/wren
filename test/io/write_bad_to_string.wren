class BadToString {
  toString { 3 }
}

IO.write(BadToString.new()) // expect: [invalid toString]
IO.print
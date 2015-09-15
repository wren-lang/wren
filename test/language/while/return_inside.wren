var f = Fn.new {
  while (true) {
    var i = "i"
    return i
  }
}

System.print(f.call())
// expect: i

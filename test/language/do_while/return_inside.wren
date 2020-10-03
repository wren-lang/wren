var f = Fn.new {
  do {
    var i = "i"
    return i
  } while(true)
}

System.print(f.call())
// expect: i

var f = new Fn {
  while (true) {
    var i = "i"
    return i
  }
}

IO.print(f.call)
// expect: i

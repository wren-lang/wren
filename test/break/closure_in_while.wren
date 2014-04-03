var f
while (true) {
  var i = "i"
  f = new Fn { IO.print(i) }
  break
}

f.call
// expect: i

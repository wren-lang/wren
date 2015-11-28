var f
while (true) {
  var i = "i"
  f = fn () { System.print(i) }
  break
}

f()
// expect: i

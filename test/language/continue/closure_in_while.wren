var f
while (f == null) {
  var i = "i"
  f = Fn.new { System.print(i) }
  continue
}

f.call()
// expect: i
